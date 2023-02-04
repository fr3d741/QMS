#include <Configuration/Configuration.h>

Configuration::Configuration(JsonNode::Ptr node)
 :_node(node){

}

bool 
Configuration::IsDebug() {
    return false;
}

std::map<QString, int>
Configuration::GetPaths() {

    std::map<QString, int> paths;
    auto json_array = _node->GetArray("paths_to_watch");
    for (auto js : json_array) {
        auto p = js->GetString("path");
        auto t = js->GetInt("type");
        paths[p] = t;
    }
    return paths;
}

QString
Configuration::GetApiKey() {

    return _node->GetString("api_key");
}
