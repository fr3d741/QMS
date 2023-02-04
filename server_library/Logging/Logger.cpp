#include <Logging/Logger.h>
#include <Utility/GeneralUtilities.h>
#include <Utility/XmlNode.h>

#include <chrono>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <thread>
#include <QTextStream>

using namespace Logging;
using namespace std::chrono_literals;

static QString
GetTimeStamp() {

    std::time_t t = std::time(nullptr);
    std::tm tm = *std::localtime(&t);

    std::stringstream stream;
    stream << std::put_time(&tm, "%c");
    return QString::fromStdString(stream.str());
}

Logger::Logger() {
    _messages = std::make_shared<std::list<QString>>();
}

void 
Logger::Start() {

    auto now = std::chrono::steady_clock::now();
    auto file = std::to_string(now.time_since_epoch().count());
    auto file_name = QString("logs/%1.mslog").arg(file.c_str());
    _logger_file.setFileName(file_name);
    _logger_file.open(QIODevice::WriteOnly);

    auto t = std::thread(&Logger::startCore, this);
    t.detach();
}

void 
Logger::Stop() {

    _stop_not_requested.store(false);
}

void
Logger::LogMessage(const QString& msg) {

    XmlNode root("entry");
    root.AddAttribute("time_stamp", GetTimeStamp());
    root.AddChild("message", msg);
    {
        std::lock_guard guard(_mutex);
        _messages->push_back(root.Dump());
    }

}

void
Logger::startCore(){

    while (_stop_not_requested.load())
    {
        std::shared_ptr<std::list<QString>> replica_messages = std::make_shared<std::list<QString>>();
        {
            std::lock_guard guard(_mutex);
            _messages.swap(replica_messages);
        }

        if (replica_messages != nullptr && !replica_messages->empty()) {
            QTextStream stream(&_logger_file);
            for (auto msg : *replica_messages) {
                stream << msg << Qt::endl;
            }
            stream.flush();
        }

        std::this_thread::sleep_for(1min);
    }
}
