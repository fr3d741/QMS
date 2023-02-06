#include <Configuration/IConfiguration.h>

#include <Factories/ConfigurationFactory.h>
#include <Factories/MessageQueueFactory.h>
#include <Factories/TcpServerFactory.h>
#include <Logging/Logger.h>
#include <Networking/RestApi.h>
#include <Utility/StreamWriter.h>
#include <FileSystem/FileSystemWatcher.h>
#include <Utility/JsonNode.h>
#include <MediaServer.h>
#include <CommonDefines.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <memory>

#include <QFile>
#include <QTextStream>
#include <QDebug>

#define TEST

#ifdef TEST

#include <Utility/GeneralUtilities.h>

class MockWriter :public IStreamWriter {

    QFile out;
    QTextStream stream;
public:
    MockWriter() :
      #ifdef _WIN32
          out("f:\\tmp\\log.txt"),
          stream(&out)
      #else
          out("/home/fred/log.txt"),
          stream(&out)
      #endif
    {
        out.open(QIODevice::WriteOnly);
    }

    void Write(const std::string& , const std::string& ) {

    }

    void Write(const QString& path, const QString& content) {

        stream << path << Qt::endl;
        stream << content << Qt::endl;
    }

    void Write(const std::string& path, const QString& content) {

        stream << QString::fromStdString(path) << Qt::endl;
        stream << content << Qt::endl;
    }

    void close() {
        out.close();
    }
};

#endif // TEST

int main()
{
    std::setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));

    auto logger = std::make_shared<Logging::Logger>();
    logger->Start();
    IConfiguration::Ptr config = ConfigurationFactory::LoadConfiguration(logger, "config.json");
    IConfiguration::Instance(config);

    RestApi::ApiKey() = config->GetApiKey();
    auto message_queue = MessageQueueFactory::CreateMessageQueue();

#ifdef TEST
    auto writer = std::make_shared<MockWriter>();
    auto server = MediaServer(logger, message_queue, writer);
#else
    auto server = MediaServer(logger, message_queue, std::make_shared<StreamWriter>());
#endif // TEST

    File_System::FileSystemWatcher fs(logger, message_queue);
    for (auto&& it : config->GetPaths()) {

        fs.AddPath(it.first);
        server.RegisterPath(it.first, it.second);
        qDebug() << it.first;

        {
            auto json = JsonNode::Create(logger);
            json->Add(KeyWords(Keys::NodeType), static_cast<int>(Actions::Path_Update));
            json->Add(KeyWords(Keys::Message), it.first);
            message_queue->Add(json->ToString());
        }
    }

    fs.Start();

    auto tcp_server = TcpServerFactory::CreateTcpServer(message_queue);
    tcp_server->ListenOn(33015);

    try {
        server.Start();
    }
    catch(...){

    }

#ifdef TEST
    writer->close();
#endif

    fs.Stop();
    tcp_server->Stop();
    logger->Stop();

    return 1;
}
