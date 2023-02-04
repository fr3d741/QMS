#include <Utility/JsonNode.h>
#include <Utility/GeneralUtilities.h>

#include <list>
#include <map>
#include <variant>
#include <limits>
#include <exception>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QList>


static QList<QJsonValue>
FindKeyRecursive(QJsonObject object, const char* key){
    QList<QJsonValue> result;
    if (object.contains(key))
        result.push_back(object.value(key));

    for(auto it = object.begin(); it != object.end(); ++it){
        if (it->isObject())
            result.append(FindKeyRecursive(it->toObject(), key));
    }
    return result;
}

class JsonNodeImpl {
public:
    QJsonObject json_object;
    QJsonDocument json_doc;
    QJsonValue json_value;

    JsonNodeImpl(QString json_string){
        json_doc = QJsonDocument::fromJson(json_string.toUtf8());
        json_object = json_doc.object();
    }

    JsonNodeImpl(QJsonObject obj){
        json_object = obj;
    }

    JsonNodeImpl(QJsonValue value){
        json_value = value;
    }

};

//class CleanJsonNodeImpl : public JsonNodeImpl {
//public:
//    CleanJsonNodeImpl();
//};

//class ImportedJsonNodeImpl : public JsonNodeImpl {
//    Logging::ILogger::Ptr _logger;
//public:
//    Json _json;

//    ImportedJsonNodeImpl(Logging::ILogger::Ptr logger, Json json) :_logger(logger), _json(json) { }

//    void Add(const std::string&, Value) {}

//    bool Has(const std::string& key) {
//        return _json.contains(key);
//    }

//    Value Get(const std::string& key) {

//        if (_json.is_array() && !_json.contains(key)) {
//            //this as array without key elements, like ["1","2"]
//            auto js_size = _json.size();
//            std::vector<std::string> nodes;
//            nodes.reserve(js_size);
//            for(auto i = 0; i < js_size; ++i)
//                nodes.push_back(_json[i]);
//            return nodes;
//        }
//        try {
//            auto&& json = _json[key];

//            if (json.is_number_integer())
//                return json.get<int>();
//            if (json.is_number_float())
//                return json.get<double>();
//            if (json.is_boolean())
//                return json.get<bool>();
//            if (json.is_string())
//                return json.get<std::string>();
//            if (json.is_object())
//                return JsonNode::Create(std::make_shared<ImportedJsonNodeImpl>(_logger, json.get<Json>()));
//            if (json.is_array())
//            {
//                auto jsons = json.get<std::vector<Json>>();
//                std::vector<JsonNode::Ptr> nodes;
//                nodes.reserve(jsons.size());
//                for (auto js : jsons)
//                    nodes.push_back(JsonNode::Create(std::make_shared<ImportedJsonNodeImpl>(_logger, js)));
//                return nodes;
//            }
//        }
//        catch (std::exception e) {
//            _logger->LogMessage(e.what());
//        }
        
//        return  Value();
//    }

//    Value Get() {

//        if (_json.is_boolean())
//            return _json.get<bool>();
//        if (_json.is_number_integer())
//            return _json.get<int>();
//        if (_json.is_number_float())
//            return _json.get<double>();
//        if (_json.is_string())
//            return _json.get<std::string>();
//        if (_json.is_array())
//            return _json.get<std::vector<std::string>>();
//        if (_json.is_object())
//            return JsonNode::Create(std::make_shared<ImportedJsonNodeImpl>(_logger, _json.get<Json>()));

//        return  Value();
//    }

//    Json Build() {
//        return _json;
//    }

//    std::list<std::string> FindTagValuesRecursive(Json& json, const std::string& key) {

//        if (json.contains(key))
//            return { json[key].get<std::string>() };

//        std::list<std::string> result;
//        if (json.is_array()) {
            
//            for (auto i = 0; i < json.size(); ++i) {
//                result.merge(FindTagValuesRecursive(json[i], key));
//            }
//        }

//        if (json.is_object()) {
//            for (auto it : json) {
//                result.merge(FindTagValuesRecursive(it, key));
//            }
//        }

//        return result;
//    }

//    std::list<std::string> FindTagValues(const std::string& key) override {

//        return FindTagValuesRecursive(_json, key);
//    }
//};

JsonNode::JsonNode(Logging::ILogger::Ptr logger)
    : _impl(nullptr)
    , _logger(logger) {

}

//void
//JsonNode::Add(const std::string& key, int value) {

//    //_impl->Add(key, value);
//}

//void
//JsonNode::Add(const std::string& key, const std::string& value) {

////    _impl->Add(key, value);
//}

//void
//JsonNode::Add(const std::string& key, double value) {

//  //  _impl->Add(key, value);
//}

//void
//JsonNode::Add(const std::string& key, bool value) {

//    _impl->Add(key, value);
//}

//void
//JsonNode::Add(const std::string& key, Ptr value) {

//    _impl->Add(key, value);
//}

//bool
//JsonNode::Get(const char* key, int& value) {

//    auto v = _impl->json_object.value(key);
//    try
//    {
//        value = std::get<int>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        return false;
//    }
//    return true;
//}

//bool
//JsonNode::Get(const std::string& key, std::string& value) {

//    auto v = _impl->Get(key);
//    try
//    {
//        value = std::get<std::string>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        return false;
//    }
//    return true;
//}

//bool
//JsonNode::Get(const std::string& key, double& value) {

//    auto v = _impl->Get(key);
//    try
//    {
//        value = std::get<double>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        return false;
//    }
//    return true;
//}

//bool
//JsonNode::Get(const std::string& key, bool& value) {

//    auto v = _impl->Get(key);
//    try
//    {
//        value = std::get<bool>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        return false;
//    }
//    return true;
//}

//bool
//JsonNode::Get(const std::string& key, Ptr& value) {

//    auto v = _impl->Get(key);
//    try
//    {
//        value = std::get<Ptr>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        return false;
//    }
//    return true;
//}

bool
JsonNode::Get(const char* key, std::vector<Ptr>& value) {

    if (_impl->json_object.contains(key) == false)
        return false;

    value = GetArray(key);
    return true;
}

int
JsonNode::GetInt(const char* key) {

    auto v = _impl->json_object.value(key);
    if (v.isUndefined())
        return std::numeric_limits<int>::infinity();

    return v.toInt();
}

QString
JsonNode::GetString(const char* key) {

    auto v = _impl->json_object.value(key);
    if (v.isUndefined())
        return "";
    return v.toString();
}

JsonNode::Ptr
JsonNode::GetPointer(const char* key) {

    auto v = _impl->json_object.value(key);
    if (v.isUndefined())
        return nullptr;

    auto node = JsonNode::Create(std::make_shared<JsonNodeImpl>(v.toObject()));
    return node;
}

std::vector<JsonNode::Ptr>
JsonNode::GetArray(const char* key) {

    auto v = _impl->json_object.value(key);
    if (v.isUndefined())
        return std::vector<JsonNode::Ptr>();

    std::vector<JsonNode::Ptr> vect;
    for( auto a : v.toArray()){

        if (a.isObject() == false)
            vect.push_back(JsonNode::Create(std::make_shared<JsonNodeImpl>(a.toObject())));
        else
            vect.push_back(JsonNode::Create(std::make_shared<JsonNodeImpl>(a)));
    }

    return vect;
}

int 
JsonNode::GetInt() {

    return _impl->json_value.toInt();
}

QString
JsonNode::GetString() {

    return _impl->json_value.toString();
}

//double
//JsonNode::GetDouble() {

//    auto v = _impl->Get();
//    try
//    {
//        return std::get<double>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        //TODO: logging
//    }
//    return std::numeric_limits<double>::quiet_NaN();
//}

//bool
//JsonNode::GetBool() {

//    auto v = _impl->Get();
//    try
//    {
//        return std::get<bool>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        // TODO:
//        throw std::invalid_argument("");
//    }
//}

//JsonNode::Ptr
//JsonNode::GetPointer() {

//    auto v = _impl->Get();
//    try
//    {
//        return std::get<JsonNode::Ptr>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        //TODO: logging
//    }
//    return nullptr;
//}

//std::vector<JsonNode::Ptr>
//JsonNode::GetArray() {

//    auto v = _impl->Get();
//    try
//    {
//        return std::get<std::vector<JsonNode::Ptr>>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        //TODO: logging
//    }
//    return {};
//}

//std::vector<std::string>
//JsonNode::GetArrayOfString() {

//    auto v = _impl->Get();
//    try
//    {
//        return std::get<std::vector<std::string>>(v);
//    }
//    catch (const std::bad_variant_access&)
//    {
//        //TODO: logging
//    }
//    return {};
//}

//QString
//JsonNode::GetAsString(const char* key) {

//    auto v = _impl->Get(key);
//    try
//    {
//        //variant<int, std::string, double, bool, JsonNode::Ptr, std::vector<JsonNode::Ptr>, std::vector<std::string>> Value;
//        switch (v.index()) {
//        case 0:
//            return std::to_string(std::get<int>(v));
//        case 1:
//            return std::get<std::string>(v);
//        case 2:
//            return std::to_string(std::get<double>(v));
//        default:
//            return "";
//        }
//    }
//    catch (const std::bad_variant_access&)
//    {
//        //TODO: logging
//    }
//    return "";
//}

bool 
JsonNode::Has(const char* key) {
    return _impl->json_object.contains(key);
}

QString
JsonNode::ToString() {

    if (!_impl->json_object.isEmpty()){
        QJsonDocument doc(_impl->json_object);
        return QString(doc.toJson());
    }

    if (!_impl->json_value.isUndefined()){
        QJsonDocument doc({_impl->json_value});
        return QString(doc.toJson());
    }

    return "";
}

std::vector<QString>
JsonNode::FindTagRecursive(const char* key) {

    auto result = FindKeyRecursive(_impl->json_object, key);
    std::vector<QString> ret;
    ret.reserve(result.size());
    for(auto it : result)
        ret.push_back(it.toString());
    //ret.insert(ret.end(), result.begin(), result.end());
    return ret;
}

JsonNode::Ptr
JsonNode::Parse(Logging::ILogger::Ptr logger, const QString& json) {

    Ptr node = Create(logger);
    node->_impl = std::make_shared<JsonNodeImpl>(json);

    return node;
}

JsonNode::Ptr
JsonNode::Create(Logging::ILogger::Ptr logger) {
    return std::shared_ptr<JsonNode>(new JsonNode(logger));
}

JsonNode::Ptr
JsonNode::Create(std::shared_ptr<JsonNodeImpl> ptr) {
    auto node = std::shared_ptr<JsonNode>(new JsonNode(nullptr));
    node->_impl = ptr;
    return node;
}
