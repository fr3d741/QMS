#pragma once

#include <Networking/ITcpServer.h>
#include <Utility/IMessageQueue.h>

#include <memory>
#include <atomic>

namespace Logging {
    class ILogger;
}

namespace Networking {

    class TcpServer : public ITcpServer
    {
        std::atomic<bool> _stop_not_requested = true;
        std::weak_ptr<Logging::ILogger> _logger;
        IMessageQueue::Ptr _messageQueue;

    public:
        TcpServer(std::weak_ptr<Logging::ILogger> logger, IMessageQueue::Ptr messageQueue);

        void ListenOn(unsigned short port) override;
        void Stop() override;
    private:
        int listenCore(unsigned short port);
    };

}
