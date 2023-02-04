#include <Utility/MessageQueue.h>

#include <mutex>

void 
MessageQueue::Add(const QString& msg) {

    std::unique_lock<std::shared_mutex> l(_mtx);
    _queue.push(msg);
}

QString
MessageQueue::Pop() {

    std::unique_lock<std::shared_mutex> l(_mtx);
    auto val = _queue.front();
    _queue.pop();
    return val;
}

std::queue<QString>
MessageQueue::BulkPop() {

    std::queue<QString> queue;
    {
        std::unique_lock<std::shared_mutex> l(_mtx);
        std::swap(queue, _queue);
    }

    return queue;
}

bool 
MessageQueue::HasMessage() const {

    std::shared_lock<std::shared_mutex> lock(_mtx);
    return !_queue.empty();
}

