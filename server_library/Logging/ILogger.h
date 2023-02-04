#pragma once

#include "Macros.h"
#include <CommonDefines.h>

#include <memory>

namespace Logging {

    class DLL ILogger {
    public:
        typedef std::shared_ptr<ILogger> Ptr;

        virtual void LogMessage(const QString& msg) = 0;
    };
}
