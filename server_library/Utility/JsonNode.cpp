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

    JsonNodeImpl(){}

    JsonNodeImpl(QString json_string){
        json_doc = QJsonDocument::fromJson(json_string.toUtf8());
        json_object = json_doc.object();
    }

    JsonNodeImpl(QJsonObject obj){
        json_object = obj;
    }

    JsonNodeImpl(QJsonValue value){
        json_value = value;
        if (json_value.isObject())
            json_object = json_value.toObject();
    }

};

JsonNode::JsonNode(Logging::ILogger::Ptr logger)
    : _impl(std::make_shared<JsonNodeImpl>())
    , _logger(logger) {

}

void
JsonNode::Add(const QString& key, int value) {

    _impl->json_object.insert(key, QJsonValue(value));
}

void
JsonNode::Add(const QString& key, QString value) {

    _impl->json_object.insert(key, QJsonValue(value));
}

void
JsonNode::Add(const QString& key, QStringList array){

    auto jsn_array = QJsonArray::fromStringList(array);
    _impl->json_object.insert(key, jsn_array);
}

void
JsonNode::Add(const QString& key, const std::vector<Ptr>& array){

    QJsonArray json_array;
    for(auto ptr : array){
        QJsonValue val(ptr->_impl->json_object);
        json_array.append(val);
    }
    _impl->json_object.insert(key, json_array);
}

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
    auto json_array = v.toArray();
    for(auto it = json_array.begin(); it != json_array.end(); ++it){

        if (it->isObject() == false)
            vect.push_back(JsonNode::Create(std::make_shared<JsonNodeImpl>(it->toObject())));
        else
            vect.push_back(JsonNode::Create(std::make_shared<JsonNodeImpl>(*it)));
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
