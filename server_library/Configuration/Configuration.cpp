#include <Configuration/Configuration.h>

static void
GetPaths(JsonNode::Ptr node, std::map<QString, int>& paths) {

    if (node->Has("paths_to_watch") == false)
        return;

    auto json_array = node->GetArray("paths_to_watch");
    for (auto js : json_array) {
        auto p = js->GetString("path");
        auto t = js->GetInt("type");
        paths[p] = t;
    }
}

static void
GetLogLevel(JsonNode::Ptr node, int& log_level) {

    if (node->Has("LogLevel") == false)
        return;

    log_level = node->GetInt("LogLevel");
}

Configuration::Configuration(JsonNode::Ptr node)
 :_node(node) {
    ::GetPaths(_node, _paths);
    ::GetLogLevel(_node, _log_level);
}

std::map<QString, int>
Configuration::GetPaths() {

    return _paths;
}

QString
Configuration::GetApiKey() {

    return _node->GetString("api_key");
}
