#pragma once

#include "Macros.h"
#include <Utility/IMessageQueue.h>
#include <Networking/ITcpServer.h>

#include <memory>

class DLL TcpServerFactory {
public:
    static Networking::ITcpServer::Ptr CreateTcpServer(IMessageQueue::Ptr messageQueue);
};
