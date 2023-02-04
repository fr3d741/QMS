#pragma once

#include "Macros.h"
#include <Logging/ILogger.h>

#include <list>
#include <string>
#include <memory>
#include <mutex>
#include <fstream>

#include <QString>

namespace Logging {

    class DLL Logger : public ILogger
    {
        std::ofstream _logger_file;
        std::mutex _mutex;
        std::shared_ptr<std::list<QString>> _messages;
    public:
        Logger();

        void Start();
        void Stop();

        void LogMessage(const char* msg) override;
        void LogMessage(const std::string& msg) override;
        void LogMessage(const QString& msg) override;
    };

}
