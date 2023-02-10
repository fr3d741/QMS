#pragma once

#include "Macros.h"
#include <Configuration/IConfiguration.h>
#include <Utility/JsonNode.h>
#include <Logging/ILogger.h>

class DLL Configuration : public IConfiguration
{
    std::map<QString, int> _paths;
    int _log_level = 0;
    JsonNode::Ptr _node;
    Logging::ILogger::Ptr _logger;
public:
    Configuration(JsonNode::Ptr node, Logging::ILogger::Ptr logger);

    const std::map<QString, int>& GetPaths() override;
    QString GetApiKey() override;
    bool LoadConfiguration(JsonNode::Ptr json) override;
    QString DumpConfiguration() override;
};

