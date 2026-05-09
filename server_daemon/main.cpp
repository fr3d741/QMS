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



#include <Utility/GeneralUtilities.h>

class DummyWriter :public IStreamWriter {

    QFile out;
    QTextStream stream;
public:
    DummyWriter() :
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

int main(int argc, char** argv)
{
    std::setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));

    auto logger = std::make_shared<Logging::Logger>();
    logger->Start();
    IConfiguration::Ptr config = ConfigurationFactory::LoadConfiguration(logger, "config.json");
    IConfiguration::Instance(config);

    logger->SetLogLevel(static_cast<Logging::LogLevel>(config->GetLogLevel()));
    RestApi::ApiKey() = config->GetApiKey();
    auto message_queue = MessageQueueFactory::CreateMessageQueue();

    auto tcp_server = TcpServerFactory::CreateTcpServer(message_queue);
#ifdef _DEBUG
    auto writer = std::make_shared<DummyWriter>();
   // auto server = MediaServer(logger, message_queue, writer, tcp_server);
#else

#endif // TEST

    //File_System::FileSystemWatcher fs(logger, message_queue);

    //fs.Start();

    //tcp_server->ListenOn(33015);

    enum Param{
        UpdateAll,
        SinglePath,
        UpdateType,
        ScanAmbigious

    };

    Param options = UpdateAll;
    int type = -1; // 0 - movie, 1 - twshow

    if (1 < argc){

        for(char* c = argv[1]; *c; ++c){
            if (*c < '0' || '9' < *c){
                options = SinglePath;
                break;
            }
        }

        if (options == UpdateAll){

            type = std::stoi(argv[1]);
            switch(type){
                case 0:
                case 1:
                    options = UpdateType;
                    break;
                case 2:
                    options = ScanAmbigious;
                    break;
                default:
                    break;
            }
        }
    }

    std::function<void(MediaServer&)> func;
    IStreamWriter::Ptr stream_writer = std::make_shared<StreamWriter>();

    switch(options){
        case UpdateType:
            func = [&](MediaServer& s){
                for (auto&& it : config->GetPaths()) {

                    if (it.second != type)
                        continue;

                    auto json = JsonNode::Create(logger);
                    json->Add(KeyWords(Keys::NodeType), static_cast<int>(Actions::Rescan));
                    json->Add(KeyWords(Keys::Message), it.first);
                    message_queue->Add(json->ToString());
                    qDebug() << it.first;
                }

                s.ProcessOnce();
            };
            break;
        case SinglePath:
            func = [&](MediaServer& s){ s.UpdateSinglePath(argv[1]);};
            break;
        case UpdateAll:
            func = [&](MediaServer& s)
            {
                auto json = JsonNode::Create(logger);
                json->Add(KeyWords(Keys::NodeType), static_cast<int>(Actions::Path_Update));
                message_queue->Add(json->ToString());
                s.ProcessOnce();
            };
            break;
        case ScanAmbigious:
            func = [&](MediaServer& s){
                for (auto&& it : config->GetPaths()) {

                auto json = JsonNode::Create(logger);
                json->Add(KeyWords(Keys::NodeType), static_cast<int>(Actions::Rescan));
                json->Add(KeyWords(Keys::Message), it.first);
                message_queue->Add(json->ToString());
                qDebug() << it.first;
            }

                s.ProcessOnce();
            };
            stream_writer = std::make_shared<DummyWriter>();
            break;
        default:
            break;
    }

    auto server = MediaServer(logger, message_queue, stream_writer, tcp_server);
    try{
        func(server);
    }
    catch(...){

    }

#ifdef _DEBUG
    writer->close();
#endif

    //fs.Stop();
    //tcp_server->Stop();
    logger->Stop();

    return 1;
}
