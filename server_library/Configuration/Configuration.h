#pragma once

#include "Macros.h"
#include <Configuration/IConfiguration.h>
#include <Utility/JsonNode.h>

class DLL Configuration : public IConfiguration
{
    std::map<QString, int> _paths;
    int _log_level = 0;
    JsonNode::Ptr _node;
public:
    Configuration(JsonNode::Ptr node);

    std::map<QString, int> GetPaths() override;
    QString GetApiKey() override;
};

