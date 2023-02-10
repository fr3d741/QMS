#pragma once

#include <QString>

#include <memory.h>

namespace Networking {

    class ITcpServer
    {
    public:
        typedef std::shared_ptr<ITcpServer> Ptr;
        virtual ~ITcpServer() = default;

        virtual void ListenOn(unsigned short port) = 0;
        virtual void Stop() = 0;
        virtual void SendResponse(QString msg) = 0;
    };
}
