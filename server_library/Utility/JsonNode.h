#pragma once

#include "Macros.h"
#include <Logging/ILogger.h>
#include <CommonDefines.h>

#include <memory>
#include <vector>

class JsonNodeImpl;

class DLL JsonNode {

    std::shared_ptr<JsonNodeImpl> _impl;
    JsonNode();

public:
    typedef std::shared_ptr<JsonNode> Ptr;

    void Add(const QString& key, int value);
    void Add(const QString& key, QString value);
    void Add(const QString& key, QStringList array);
    void Add(const QString& key, const std::vector<Ptr>& array);
    bool Get(const char* key, std::vector<Ptr>& value);

    int GetInt(const char* key);
    QString GetString(const char* key);
    Ptr GetPointer(const char* key);
    std::vector<Ptr> GetArray(const char* key);

    int GetInt();
    QString GetString();

    bool Has(const char* key);

    QString ToString();

    // Utilities
    std::vector<QString> FindTagRecursive(const char* key);

    // static classes
    static Ptr Parse(Logging::ILogger::Ptr logger, const QString& json);
    static Ptr Create(Logging::ILogger::Ptr logger = nullptr);
    static Ptr Create(std::shared_ptr<JsonNodeImpl> ptr);
};
