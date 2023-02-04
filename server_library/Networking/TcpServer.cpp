#include <Networking/TcpServer.h>

#include <thread>
#include <iostream>
#include <chrono>

#include <QTcpServer>
#include <QTcpSocket>

using namespace std::chrono_literals;
using namespace Networking;

static QString Process(QTcpSocket* socket){

    if (socket->waitForReadyRead() == false)
        return "";

    if (socket->bytesAvailable() < 1)
        return "";

    auto string = QString(socket->readAll());
    return string;
}

TcpServer::TcpServer(std::weak_ptr<Logging::ILogger> logger, IMessageQueue::Ptr messageQueue)
    : _logger(logger)
    , _messageQueue(messageQueue) {

}

void TcpServer::ListenOn(unsigned short port) {

    auto t = std::thread(&TcpServer::listenCore, this, port);
    t.detach();
}

void
TcpServer::Stop(){
    _stop_not_requested.store(false);
}

int TcpServer::listenCore(unsigned short port) {

    QTcpServer server;
    server.listen(QHostAddress::Any, port);

    while(_stop_not_requested.load()){

        if (server.hasPendingConnections()){

            auto result = Process(server.nextPendingConnection());
            if (result.isEmpty())
                continue;

            _messageQueue->Add(result);
            continue;
        }

        std::this_thread::sleep_for(1s);
    }

    return 0;
}
