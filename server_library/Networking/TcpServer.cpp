#include <Networking/TcpServer.h>
#include <Utility/timer.h>

#include <thread>
#include <iostream>
#include <chrono>

#include <QTcpServer>
#include <QTcpSocket>

using namespace std::chrono_literals;
using namespace Networking;

static QString
process(QTcpSocket* socket){

    if (socket->waitForReadyRead() == false)
        return "";

    if (socket->bytesAvailable() < 1)
        return "";

    auto string = QString(socket->readAll());
    return string;
}

static bool
send(const QString& msg, QTcpSocket* socket){

    if (socket->isValid() == false)
        return false;

    socket->write(msg.toUtf8());
    socket->flush();
    return true;
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

void
TcpServer::SendResponse(QString msg){

    std::lock_guard<std::mutex> guard(_mutex);
    _outgoing.push_back(msg);
}

int TcpServer::listenCore(unsigned short port) {

    QTcpServer server;
    server.listen(QHostAddress::Any, port);
    QTcpSocket* last_active = nullptr;
    Timer timer;

    while(_stop_not_requested.load()){

        if (last_active != nullptr){

            auto err = last_active->error();
            auto elapsed = timer.ElapsedSeconds();
            if (last_active->state() != QAbstractSocket::ConnectedState || -1 < err || 60 <= elapsed)
            {
                last_active->close();
                last_active->deleteLater();
                last_active = nullptr;
                continue;
            }

            if (last_active->bytesAvailable()){
                auto result = process(last_active);
                if (result.isEmpty())
                    continue;

                _messageQueue->Add(result);
                continue;
            }

            std::list<QString> outgoing_replica;
            {
                std::lock_guard<std::mutex> guard(_mutex);
                if (_outgoing.empty())
                    continue;

                outgoing_replica.swap(_outgoing);
            }

            for(auto&& msg : outgoing_replica){
                send(msg, last_active);
            }
            continue;
        }

        {
            std::lock_guard<std::mutex> guard(_mutex);
            if (!_outgoing.empty())
            {
                _outgoing.clear();
            }
        }

        if (server.waitForNewConnection(1000)){

            last_active = server.nextPendingConnection();
            timer.ReStart();
            auto result = process(last_active);
            if (result.isEmpty())
                continue;

            _messageQueue->Add(result);
            continue;
        }

        //std::this_thread::sleep_for(1s);
    }

    return 0;
}
