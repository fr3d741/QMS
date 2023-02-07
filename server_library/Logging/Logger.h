#pragma once

#include "Macros.h"
#include <Logging/ILogger.h>

#include <list>
#include <string>
#include <memory>
#include <mutex>
#include <fstream>

#include <QString>
#include <QFile>

namespace Logging {

    class DLL Logger : public ILogger
    {
        LogLevel _current_logLevel = LogLevel::Error;
        QFile _logger_file;
        std::atomic<bool> _stop_not_requested = true;
        std::mutex _mutex;
        std::shared_ptr<std::list<QString>> _messages;
    public:
        Logger();

        void Start();
        void Stop();

        void LogMessage(const QString& msg, LogLevel level) override;
        void SetLogLevel(LogLevel log_level) override;
        void IncreaseLogLevel() override;
        void DecreaseLogLevel() override;
    private:
        void startCore();
    };

}
