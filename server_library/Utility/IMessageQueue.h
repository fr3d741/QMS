#pragma once

#include "Macros.h"
#include <string>
#include <memory>
#include <queue>

#include <QString>

class DLL IMessageQueue {
public:
    typedef std::shared_ptr<IMessageQueue> Ptr;

    virtual void Add(const QString& msg) = 0;
    virtual QString Pop() = 0;
    virtual std::queue<QString> BulkPop() = 0;
    virtual bool HasMessage() const = 0;
};
