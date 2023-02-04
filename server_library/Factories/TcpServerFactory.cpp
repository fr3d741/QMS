#include <Factories/TcpServerFactory.h>

#include <Networking/ITcpServer.h>
#ifdef _WIN64
    #include <Networking/TcpServer.h>
#endif // _WIN64

#include <memory>

std::unique_ptr<Networking::ITcpServer> 
TcpServerFactory::CreateTcpServer(IMessageQueue::Ptr messageQueue) {

    return std::make_unique<Networking::TcpServer>(std::weak_ptr<Logging::ILogger>(), messageQueue);
}
    
