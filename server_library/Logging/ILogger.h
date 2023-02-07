#pragma once

#include "Macros.h"
#include <CommonDefines.h>

#include <memory>

namespace Logging {

    enum class LogLevel{
        Error,
        Debug,
        Detailed,
        All
    };


    class DLL ILogger {
    public:
        typedef std::shared_ptr<ILogger> Ptr;

        virtual void LogMessage(const QString& msg, LogLevel level) = 0;
        virtual void SetLogLevel(LogLevel log_level) = 0;
        virtual void IncreaseLogLevel() = 0;
        virtual void DecreaseLogLevel() = 0;

        inline void Error(const QString& msg) { LogMessage(msg, LogLevel::Error); }
        inline void DebugMsg(const QString& msg) { LogMessage(msg, LogLevel::Debug); }
    };
}
