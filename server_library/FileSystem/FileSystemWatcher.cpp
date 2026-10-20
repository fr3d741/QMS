#include <FileSystem/FileSystemWatcher.h>
#include <Configuration/IConfiguration.h>
#include <Utility/Messagebuilder.h>

#ifdef __linux__
#include <sys/fanotify.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <unistd.h>
#include <climits>
#include <cerrno>
#include <cstring>
#include <chrono>

#include <QFile>
#include <QFileInfo>
#endif

using namespace File_System;

#ifdef __linux__
static bool
IsValidFormat(QString path){

    static std::set<QString> formats = IConfiguration::Instance().GetFormats();
    return std::any_of(formats.begin(), formats.end(), [&](QString f){ return path.endsWith(f); });
}
#endif

FileSystemWatcher::FileSystemWatcher(Logging::ILogger::Ptr logger, IMessageQueue::Ptr message_queue)
    : _logger(logger)
    , _queue(message_queue) {

}

FileSystemWatcher::~FileSystemWatcher() {

    Stop();
}

void
FileSystemWatcher::AddPath(const QString& path) {

    _paths.append(path);
}

void
FileSystemWatcher::AddPath(const std::vector<QString>& paths) {

    for(auto& p : paths)
        _paths.append(p);
}

void
FileSystemWatcher::Start() {

#ifdef __linux__
    if(_running)
        return;

    // FAN_MARK_FILESYSTEM and FAN_UNLIMITED_QUEUE require CAP_SYS_ADMIN,
    // FAN_REPORT_DFID_NAME requires kernel 5.9+
    _fanotify_fd = fanotify_init(FAN_CLASS_NOTIF | FAN_REPORT_DFID_NAME | FAN_UNLIMITED_QUEUE | FAN_NONBLOCK | FAN_CLOEXEC,
                                 O_RDONLY | O_LARGEFILE);
    if(_fanotify_fd < 0) {
        _logger->Error(QString("fanotify_init failed: %1").arg(strerror(errno)));
        return;
    }

    const uint64_t mask = FAN_CREATE | FAN_DELETE | FAN_MODIFY | FAN_CLOSE_WRITE
                        | FAN_MOVED_FROM | FAN_MOVED_TO | FAN_ONDIR;

    for(auto& path : _paths) {
        const QString root = QFileInfo(path).canonicalFilePath();
        if(root.isEmpty()) {
            _logger->Error(QString("Watched path does not exist: %1").arg(path));
            continue;
        }

        const QByteArray native = QFile::encodeName(root);

        struct statfs fs;
        if(statfs(native.constData(), &fs) < 0) {
            _logger->Error(QString("statfs failed for %1: %2").arg(root, strerror(errno)));
            continue;
        }

        int fsid[2];
        static_assert(sizeof(fsid) == sizeof(fs.f_fsid), "unexpected fsid size");
        memcpy(fsid, &fs.f_fsid, sizeof(fsid));

        // the whole file system is marked once, events are filtered by _roots later
        if(FindMountFd(fsid) < 0) {
            if(fanotify_mark(_fanotify_fd, FAN_MARK_ADD | FAN_MARK_FILESYSTEM, mask, AT_FDCWD, native.constData()) < 0) {
                _logger->Error(QString("fanotify_mark failed for %1: %2").arg(root, strerror(errno)));
                continue;
            }

            const int mount_fd = open(native.constData(), O_RDONLY | O_CLOEXEC);
            if(mount_fd < 0) {
                _logger->Error(QString("open failed for %1: %2").arg(root, strerror(errno)));
                continue;
            }
            _file_systems.push_back({{fsid[0], fsid[1]}, mount_fd});
        }

        _roots.append(root);
    }

    if(_roots.isEmpty()) {
        _logger->Error("FileSystemWatcher: no path could be watched");
        Stop();
        return;
    }

    _running = true;
    _thread = std::thread(&FileSystemWatcher::WatchLoop, this);
#endif
}

void
FileSystemWatcher::Stop() {

#ifdef __linux__
    {
        std::lock_guard<std::mutex> lock(_mutex);
        _running = false;
    }
    _cv.notify_all();

    if(_thread.joinable())
        _thread.join();

    for(auto& fs : _file_systems)
        close(fs.mount_fd);
    _file_systems.clear();
    _roots.clear();

    if(_fanotify_fd >= 0) {
        close(_fanotify_fd);
        _fanotify_fd = -1;
    }
#endif
}

#ifdef __linux__
void
FileSystemWatcher::WatchLoop() {

    constexpr auto check_interval = std::chrono::seconds(10);
    alignas(fanotify_event_metadata) char buffer[8192];

    while(_running) {
        {
            std::unique_lock<std::mutex> lock(_mutex);
            _cv.wait_for(lock, check_interval, [this] { return !_running; });
        }
        if(!_running)
            break;

        // drain every event queued since the last check
        for(;;) {
            ssize_t len = read(_fanotify_fd, buffer, sizeof(buffer));
            if(len <= 0) {
                if(len < 0 && errno != EAGAIN && errno != EINTR)
                    _logger->Error(QString("fanotify read failed: %1").arg(strerror(errno)));
                break;
            }

            for(auto* event = reinterpret_cast<const fanotify_event_metadata*>(buffer);
                FAN_EVENT_OK(event, len);
                event = FAN_EVENT_NEXT(event, len)) {

                if(event->vers != FANOTIFY_METADATA_VERSION) {
                    _logger->Error("fanotify metadata version mismatch");
                    return;
                }

                if(event->mask & FAN_Q_OVERFLOW) {
                    _logger->Error("fanotify queue overflow, events were lost");
                    continue;
                }

                const QString path = ResolvePath(event);
                if(path.isEmpty() || !IsWatched(path) || !IsValidFormat(path))
                    continue;

                _queue->Add(MessageBuilder::Create(Actions::Rescan).Message(it.first).Build());
            }
        }
    }
}

int
FileSystemWatcher::FindMountFd(const int fsid[2]) const {

    for(auto& fs : _file_systems)
        if(fs.fsid[0] == fsid[0] && fs.fsid[1] == fsid[1])
            return fs.mount_fd;
    return -1;
}

QString
FileSystemWatcher::ResolvePath(const void* raw_event) const {

    const auto* event = static_cast<const fanotify_event_metadata*>(raw_event);
    const char* ptr = reinterpret_cast<const char*>(event) + event->metadata_len;
    const char* end = reinterpret_cast<const char*>(event) + event->event_len;

    while(ptr < end) {
        const auto* header = reinterpret_cast<const fanotify_event_info_header*>(ptr);
        if(header->len == 0)
            break;

        if(header->info_type == FAN_EVENT_INFO_TYPE_DFID_NAME || header->info_type == FAN_EVENT_INFO_TYPE_DFID) {
            const auto* fid = reinterpret_cast<const fanotify_event_info_fid*>(header);
            auto* handle = reinterpret_cast<file_handle*>(const_cast<unsigned char*>(fid->handle));

            int fsid[2];
            memcpy(fsid, &fid->fsid, sizeof(fsid));
            const int mount_fd = FindMountFd(fsid);
            if(mount_fd < 0)
                return {};

            // fails with ESTALE when the directory itself is already gone
            const int dir_fd = open_by_handle_at(mount_fd, handle, O_PATH | O_CLOEXEC);
            if(dir_fd < 0)
                return {};

            char dir_path[PATH_MAX];
            const QByteArray proc_link = QByteArray("/proc/self/fd/") + QByteArray::number(dir_fd);
            const ssize_t dir_len = readlink(proc_link.constData(), dir_path, sizeof(dir_path) - 1);
            close(dir_fd);
            if(dir_len < 0)
                return {};

            QString path = QFile::decodeName(QByteArray(dir_path, static_cast<int>(dir_len)));

            if(header->info_type == FAN_EVENT_INFO_TYPE_DFID_NAME) {
                const char* name = reinterpret_cast<const char*>(handle->f_handle) + handle->handle_bytes;
                if(strcmp(name, ".") != 0)
                    path += (path.endsWith('/') ? "" : "/") + QFile::decodeName(name);
            }
            return path;
        }

        ptr += header->len;
    }

    return {};
}

bool
FileSystemWatcher::IsWatched(const QString& path) const {

    for(auto& root : _roots) {
        if(root == "/" || path == root || path.startsWith(root + '/'))
            return true;
    }
    return false;
}
#endif
