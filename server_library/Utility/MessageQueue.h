#pragma once

#include <shared_mutex>
#include <string>
#include <queue>

#include <Utility/IMessageQueue.h>

class MessageQueue : public IMessageQueue
{
    mutable std::shared_mutex _mtx;
    std::queue<QString> _queue;
public:

    void Add(const QString& msg) override;
    QString Pop() override;
    virtual std::queue<QString> BulkPop() override;
    bool HasMessage() const override;
};

