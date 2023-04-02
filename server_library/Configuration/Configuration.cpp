#include <Configuration/Configuration.h>
#include <QFileInfo>

const char* LOG_LEVEL = "LogLevel";
const char* PATHS_TO_WATCH = "paths_to_watch";
const char* PATH = "path";
const char* TYPE = "type";
const char* API_KEY = "api_key";

static bool
GetPaths(JsonNode::Ptr node, std::map<QString, int>& paths) {

    if (node->Has(PATHS_TO_WATCH) == false)
        return false;

    bool is_changed = false;
    auto json_array = node->GetArray(PATHS_TO_WATCH);
    for (auto js : json_array) {
        auto p = js->GetString(PATH);
        auto t = js->GetInt(TYPE);
        QFileInfo info(p);
        paths[info.absoluteFilePath().toLower()] = t;
        is_changed = true;
    }

    return is_changed;
}

static bool
GetLogLevel(JsonNode::Ptr node, int& log_level) {

    if (node->Has(LOG_LEVEL) == false)
        return false;

    auto level = node->GetInt(LOG_LEVEL);
    if (level == log_level)
        return false;

    log_level = level;
    return true;
}

Configuration::Configuration(JsonNode::Ptr node, Logging::ILogger::Ptr logger)
 :_node(node)
 ,_logger(logger){
    ::GetPaths(_node, _paths);
    ::GetLogLevel(_node, _log_level);
}

const std::map<QString, int>&
Configuration::GetPaths() {

    return _paths;
}

int
Configuration::GetLogLevel(){
    return _log_level;
}

QString
Configuration::GetApiKey() {

    return _node->GetString(API_KEY);
}

bool
Configuration::LoadConfiguration(JsonNode::Ptr json){

    bool is_changed = false;
    is_changed = ::GetPaths(json, _paths) || is_changed;
    is_changed = ::GetLogLevel(json, _log_level) || is_changed;
    return is_changed;
}

QString
Configuration::DumpConfiguration(){

    auto node = JsonNode::Create(_logger);
    node->Add(LOG_LEVEL, _log_level);
    std::vector<JsonNode::Ptr> array;
    array.reserve(_paths.size());
    for(auto&& item : _paths){
        auto child = JsonNode::Create(_logger);
        child->Add(TYPE, item.second);
        child->Add(PATH, item.first);
        array.push_back(child);
    }
    node->Add(PATHS_TO_WATCH, array);
    return node->ToString();
}
