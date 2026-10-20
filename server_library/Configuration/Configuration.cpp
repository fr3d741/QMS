#include <Configuration/Configuration.h>
#include <QFileInfo>

#include <iostream>

const char* LOG_LEVEL = "LogLevel";
const char* PATHS_TO_WATCH = "paths_to_watch";
const char* PATH = "path";
const char* TYPE = "type";
const char* API_KEY = "api_key";
const char* KNOWN_FORMATS = "known_formats";

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

static bool
GetFormats(JsonNode::Ptr node, std::set<QString>& formats) {

    std::cout << "format1" << std::endl;
    if (node->Has(KNOWN_FORMATS) == false)
        return false;

    std::cout << "format2" << std::endl;
    auto format_str = node->GetString(KNOWN_FORMATS);
    if (format_str.isEmpty())
        return false;

    auto list = format_str.split(";");
    std::cout << "format3 " << list.size() << std::endl;
    auto is_changed = false;
    for (auto& i : list){
        std::cout << "format " << i.toStdString() << std::endl;
        auto result = formats.insert(i);
        if (result.second)
            is_changed = true;
    }

    return is_changed;
}

Configuration::Configuration(JsonNode::Ptr node, Logging::ILogger::Ptr logger)
    :_node(node)
    ,_logger(logger){
    ::GetPaths(_node, _paths);
    ::GetLogLevel(_node, _log_level);
    ::GetFormats(_node, _formats);
}

const std::map<QString, int>&
Configuration::GetPaths() {

    return _paths;
}

const std::vector<QString>
Configuration::GetWatchedPaths(){
    std::vector<QString> paths;
    paths.reserve(_paths.size());
    for (auto& i : _paths)
        paths.push_back(i.first);

    return paths;
}

int
Configuration::GetLogLevel(){
    return _log_level;
}

QString
Configuration::GetApiKey() {

    return _node->GetString(API_KEY);
}

const std::set<QString>&
Configuration::GetFormats(){
    return _formats;
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
