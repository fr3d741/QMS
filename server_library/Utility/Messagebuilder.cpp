#include "Messagebuilder.h"

#include <Utility/JsonNode.h>

MessageBuilder::MessageBuilder(Actions type)
    :_type(type){
}

MessageBuilder&
MessageBuilder::Message(QString content){
    _content = content;
    return *this;
}

QString
MessageBuilder::Build(){

    auto json = JsonNode::Create();
    json->Add(KeyWords(Keys::NodeType), static_cast<int>(_type));
    if (_content.isEmpty() == false)
        json->Add(KeyWords(Keys::Message), _content);

    return json->ToString();
}

MessageBuilder
MessageBuilder::Create(Actions type){
    return MessageBuilder(type);
}