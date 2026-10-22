#include <Configuration/IConfiguration.h>

#include <Factories/ConfigurationFactory.h>
#include <Factories/MessageQueueFactory.h>

#include <Logging/Logger.h>

#include <Networking/RestApi.h>

#include <FileSystem/FileSystemWatcher.h>

#include <Utility/StreamWriter.h>
#include <Utility/JsonNode.h>
#include <Utility/Messagebuilder.h>

#include <MediaServer.h>
#include <CommonDefines.h>

#include <string>
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
        if (out.open(QIODevice::WriteOnly)) return;
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

void mainCore(int argc, char** argv, Logging::ILogger::Ptr logger){

    IConfiguration::Ptr config = ConfigurationFactory::LoadConfiguration(logger, "config.json");
    IConfiguration::Instance(config);
    logger->SetLogLevel(static_cast<Logging::LogLevel>(config->GetLogLevel()));
    RestApi::ApiKey() = config->GetApiKey();
    auto message_queue = MessageQueueFactory::CreateMessageQueue();

    File_System::FileSystemWatcher fs(logger, message_queue);

    enum Param{
        UpdateAll,
        SinglePath,
        UpdateType,
        ScanAmbigious,
        MonitorBackground,

    };

    // message_queue->Add(MessageBuilder::Create(Actions::Rescan).Message("f:\\Source\\QMS\\build\\Friends (1994)\\Season 01\\S01E01.mkv").Build());
    // message_queue->Add(MessageBuilder::Create(Actions::Rescan).Message("f:\\Source\\QMS\\build\\Friends (1994)\\Season 01\\S01E02.mkv").Build());

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
                options = MonitorBackground;
                break;
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

    std::function<void(MediaServer&)> func = [&](MediaServer&){ };
    IStreamWriter::Ptr stream_writer = std::make_shared<StreamWriter>();

    switch(options){

    case UpdateType:
        func = [&](MediaServer& s){
            for (auto&& it : config->GetPaths()) {

                if (it.second != type)
                    continue;

                message_queue->Add(MessageBuilder::Create(Actions::Rescan).Message(it.first).Build());
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
            message_queue->Add(MessageBuilder::Create(Actions::Path_Update).Build());
            s.ProcessOnce();
        };
        break;
    case ScanAmbigious:
        func = [&](MediaServer& s){
            for (auto&& it : config->GetPaths()) {

                message_queue->Add(MessageBuilder::Create(Actions::Rescan).Message(it.first).Build());
                qDebug() << it.first;
            }

            s.ProcessOnce();
        };
        stream_writer = std::make_shared<DummyWriter>();
        break;
    case MonitorBackground:
    {
        fs.AddPath(config->GetWatchedPaths());
        fs.Start();
        func = [&](MediaServer& s) { s.Start(); };
    }
    break;
    default:
        break;
    }

    auto server = MediaServer(logger, message_queue, stream_writer);
    try{
        func(server);
    }
    catch(...){

    }
}

int main(int argc, char** argv)
{
    auto logger = std::make_shared<Logging::Logger>();
    logger->Start();

    try{
        mainCore(argc, argv, logger);
    }
    catch(std::exception& e){
        logger->LogMessage(QString(e.what()), Logging::LogLevel::Error);
    }

    logger->Stop();

    return 1;
}
