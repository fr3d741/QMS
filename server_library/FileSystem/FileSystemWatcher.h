#pragma once

#include "Macros.h"
#include <Utility/IMessageQueue.h>
#include <Logging/ILogger.h>

#include <QString>
#include <QStringList>

#include <vector>

#ifdef __linux__
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#endif

namespace File_System {

    class DLL IFileSystemWatcher {
    public:
        virtual void AddPath(const QString& path) = 0;
        virtual void AddPath(const std::vector<QString>& paths) = 0;
        virtual void Start() = 0;
        virtual void Stop() = 0;
    };

    class DLL FileSystemWatcher : public IFileSystemWatcher {

        Logging::ILogger::Ptr _logger;
        IMessageQueue::Ptr _queue;
        QStringList _paths;

#ifdef __linux__
        struct WatchedFileSystem {
            int fsid[2];
            int mount_fd;
        };

        int _fanotify_fd = -1;
        std::vector<WatchedFileSystem> _file_systems;
        QStringList _roots;
        std::thread _thread;
        std::atomic<bool> _running{false};
        std::mutex _mutex;
        std::condition_variable _cv;

        void WatchLoop();
        int FindMountFd(const int fsid[2]) const;
        QString ResolvePath(const void* event) const;
        bool IsWatched(const QString& path) const;
#endif

    public:
        FileSystemWatcher(Logging::ILogger::Ptr logger, IMessageQueue::Ptr queue);
        ~FileSystemWatcher();
        void AddPath(const QString& path) override;
        void AddPath(const std::vector<QString>& paths) override;
        void Start() override;
        void Stop() override;
    };

}
