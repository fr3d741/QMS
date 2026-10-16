#pragma once

#ifdef _WIN32
#include <Utility/IMessageQueue.h>
#include <Logging/ILogger.h>

#include <vector>

class MessageQueue;

namespace File_System {

    class WinFSWatcher {
    public:

        static void WatchDirectories(Logging::ILogger::Ptr logger, const std::vector<QString>& paths, IMessageQueue::Ptr queue);
    };

}
#endif
