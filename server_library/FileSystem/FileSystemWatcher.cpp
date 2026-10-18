#include <FileSystem/FileSystemWatcher.h>

//#include <QFileSystemWatcher>

using namespace File_System;

// static QFileSystemWatcher&
// getWatcher(){
//     static QFileSystemWatcher fs_watch;
//     return fs_watch;
// }

FileSystemWatcher::FileSystemWatcher(Logging::ILogger::Ptr logger, IMessageQueue::Ptr message_queue)
    : _logger(logger)
    , _queue(message_queue) {

}

void 
FileSystemWatcher::AddPath(const QString& /*path*/) {


}

void 
FileSystemWatcher::AddPath(const std::vector<QString>& /*paths*/) {

}

void 
FileSystemWatcher::Start() {

}

void 
FileSystemWatcher::Stop() {
}
