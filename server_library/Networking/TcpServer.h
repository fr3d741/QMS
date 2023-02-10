#pragma once

#include <Networking/ITcpServer.h>
#include <Utility/IMessageQueue.h>

#include <memory>
#include <atomic>
#include <list>
#include <mutex>

namespace Logging {
    class ILogger;
}

namespace Networking {

    class TcpServer : public ITcpServer
    {
        std::atomic<bool> _stop_not_requested = true;
        std::weak_ptr<Logging::ILogger> _logger;
        IMessageQueue::Ptr _messageQueue;
        std::list<QString> _outgoing;
        std::mutex _mutex;

    public:
        TcpServer(std::weak_ptr<Logging::ILogger> logger, IMessageQueue::Ptr messageQueue);

        void ListenOn(unsigned short port) override;
        void Stop() override;
        void SendResponse(QString msg) override;
    private:
        int listenCore(unsigned short port);
    };

}
