#pragma once

#include "Macros.h"
#include <memory>
#include <string>
#include <list>
#include <map>

#include <QString>

class DLL IConfiguration {

protected:
    IConfiguration() = default;
    IConfiguration(const IConfiguration&) = delete;
    IConfiguration& operator=(const IConfiguration&) = delete;
public:
    typedef std::shared_ptr<IConfiguration> Ptr;

    static IConfiguration& Instance();
    static void Instance(Ptr configuration);

    virtual bool IsDebug() = 0;
    virtual bool IsLogRequest() = 0; // TODO: refactor to log level
    virtual std::map<QString, int> GetPaths() = 0;
    virtual QString GetApiKey() = 0;
};
