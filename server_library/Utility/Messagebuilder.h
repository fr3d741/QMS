#ifndef MESSAGEBUILDER_H
#define MESSAGEBUILDER_H

#include <CommonDefines.h>

class DLL MessageBuilder
{
    Actions _type;
    QString _content;

    MessageBuilder(Actions type);
public:

    MessageBuilder& Message(QString content);

    QString Build();

    static MessageBuilder Create(Actions type);
};

#endif // MESSAGEBUILDER_H
