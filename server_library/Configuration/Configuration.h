#pragma once

#include "Macros.h"
#include <Configuration/IConfiguration.h>
#include <Utility/JsonNode.h>

class DLL Configuration : public IConfiguration
{
    bool _isDebug = true;
    bool _isLogRequest = false;
    JsonNode::Ptr _node;
public:
    Configuration(JsonNode::Ptr node);

    bool IsDebug() override;
    bool IsLogRequest() override;

    std::map<QString, int> GetPaths() override;
    QString GetApiKey() override;
};

