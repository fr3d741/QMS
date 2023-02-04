#pragma once

#include "Macros.h"
#include <Logging/ILogger.h>
#include <CommonDefines.h>

#include <memory>
#include <vector>

class JsonNodeImpl;

class DLL JsonNode {

    std::shared_ptr<JsonNodeImpl> _impl;
    Logging::ILogger::Ptr _logger;
    JsonNode(Logging::ILogger::Ptr logger);

public:
    typedef std::shared_ptr<JsonNode> Ptr;

//    void Add(const std::string& key, int value);
//    void Add(const std::string& key, const std::string& value);
//    void Add(const std::string& key, double value);
//    void Add(const std::string& key, bool value);
//    void Add(const std::string& key, Ptr value);

//    bool Get(const std::string& key, int& value);
//    bool Get(const std::string& key, std::string& value);
//    bool Get(const std::string& key, double& value);
//    bool Get(const std::string& key, bool& value);
//    bool Get(const std::string& key, Ptr& value);
    bool Get(const char* key, std::vector<Ptr>& value);

    int GetInt(const char* key);
    QString GetString(const char* key);
    Ptr GetPointer(const char* key);
    std::vector<Ptr> GetArray(const char* key);

    int GetInt();
    QString GetString();
//    double GetDouble();
//    bool GetBool();
//    Ptr GetPointer();
//    std::vector<Ptr> GetArray();

    //QString GetAsString(const char* key);

    bool Has(const char* key);

    QString ToString();

    // Utilities
    std::vector<QString> FindTagRecursive(const char* key);

    // static classes
    static Ptr Parse(Logging::ILogger::Ptr logger, const QString& json);
    static Ptr Create(Logging::ILogger::Ptr logger);
    static Ptr Create(std::shared_ptr<JsonNodeImpl> ptr);
};
