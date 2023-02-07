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

//    virtual bool LoadConfiguration(QString json) = 0;
//    virtual QString DumpConfiguration() = 0;
    virtual std::map<QString, int> GetPaths() = 0;
    virtual QString GetApiKey() = 0;
};
