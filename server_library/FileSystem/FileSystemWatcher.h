#pragma once

#include "Macros.h"
#include <Utility/IMessageQueue.h>
#include <Logging/ILogger.h>

#include <string>
#include <vector>
#include <QString>

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
        std::vector<QString> _paths;
        bool _started = false;
    public:
        FileSystemWatcher(Logging::ILogger::Ptr logger, IMessageQueue::Ptr queue);
        void AddPath(const QString& path) override;
        void AddPath(const std::vector<QString>& paths) override;
        void Start() override;
        void Stop() override;
    };

}
