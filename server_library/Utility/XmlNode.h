#pragma once

#include <CommonDefines.h>
#include <map>
#include <string>
#include <list>

class XmlNode
{
    std::map<std::string, QString> _attributes;
    std::string _element;
    QString _value;
    std::list<XmlNode> _children;
    bool _is_value_created;
public:
    XmlNode() = default; // To support STL containers
    XmlNode(const std::string& elementName);
    XmlNode(const std::string& elementName, const QString& value);

    XmlNode& AddChild(const std::string& element);
    XmlNode& AddChild(const std::string& element, const QString& value);
    //XmlNode& AddChild(const std::string& element, const std::string& value);
    void AddAttribute(const std::string& attr, const QString& value);
    //void AddAttribute(const std::string& attr, const std::string& value);

    QString Dump(int tab = 0);
};
