#include <Factories/TcpServerFactory.h>

#include <Networking/ITcpServer.h>
#include <Networking/TcpServer.h>

#include <memory>

std::shared_ptr<Networking::ITcpServer>
TcpServerFactory::CreateTcpServer(IMessageQueue::Ptr messageQueue) {

    return std::make_shared<Networking::TcpServer>(std::weak_ptr<Logging::ILogger>(), messageQueue);
}
    
