#pragma once

#include "Macros.h"
#include <memory>
#include <string>
#include <list>
#include <map>

#include <QString>

#include <Utility/JsonNode.h>

class DLL IConfiguration {

protected:
    IConfiguration() = default;
    IConfiguration(const IConfiguration&) = delete;
    IConfiguration& operator=(const IConfiguration&) = delete;
public:
    typedef std::shared_ptr<IConfiguration> Ptr;

    static IConfiguration& Instance();
    static void Instance(Ptr configuration);

    virtual bool LoadConfiguration(JsonNode::Ptr json) = 0;
    virtual QString DumpConfiguration() = 0;
    virtual const std::map<QString, int>& GetPaths() = 0;
    virtual QString GetApiKey() = 0;
    virtual int GetLogLevel() = 0;
};
