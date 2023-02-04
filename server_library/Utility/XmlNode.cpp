#include <Utility/XmlNode.h>
#include <Utility/GeneralUtilities.h>

#include <exception>
#include <stdexcept>
#include <sstream>
#include <QTextStream>

using G = GeneralUtilities;

XmlNode::XmlNode(const std::string& elementName)
: _element(elementName), _is_value_created(false) {
}

XmlNode::XmlNode(const std::string& elementName, const QString& value)
: _element(elementName), _value(value), _is_value_created(true){
}

XmlNode&
XmlNode::AddChild(const std::string& element) {

    if (_is_value_created)
        throw std::logic_error(element);

    _children.push_back(XmlNode(element));
    return _children.back();
}

XmlNode&
XmlNode::AddChild(const std::string& element, const QString& value) {

    if (_is_value_created)
        throw std::logic_error(element);

    _children.push_back(XmlNode(element, value));
    return _children.back();
}

//XmlNode&
//XmlNode::AddChild(const std::string& element, const std::string& value) {
//    return AddChild(element, QString::fromStdString(value));
//}

void
XmlNode::AddAttribute(const std::string& attr, const QString& value) {
    _attributes[attr] = value;
}

//void
//XmlNode::AddAttribute(const std::string& attr, const std::string& value) {
//    AddAttribute(attr, QString::fromStdString(value));
//}

static QString
tabs(int n) {
    return QString(n, '\t');
}

QString
XmlNode::Dump(int tab) {

    QString result;
    QTextStream stream(&result);

    stream << tabs(tab) << "<" << QString::fromStdString(_element);
    for (auto&& item : _attributes) {
        stream << " " << QString::fromStdString(item.first) << "=\"" << item.second << "\"";
    }
    stream << ">";

    if (_is_value_created)
        stream << _value << "</" << QString::fromStdString(_element) << ">" << Qt::endl;
    else {
        stream << Qt::endl;
        for (auto var : _children)
        {
            stream << var.Dump(tab + 1);
        }

        stream << tabs(tab) << "</" << QString::fromStdString(_element) << ">" << Qt::endl;
    }

    return result;
}
