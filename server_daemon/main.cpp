#pragma once

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

#define TEST

#ifdef TEST

#include <Utility/GeneralUtilities.h>

class MockWriter :public IStreamWriter {

    //std::wofstream out;
    QFile out;
    QTextStream stream;
    bool is_good = false;
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
       //out.open("f:\\tmp\\log.txt", std::ios::binary);
        //is_good = out.is_open() && out.good();
        //out.exceptions(std::wofstream::failbit);
        //out.exceptions(std::wofstream::badbit);
    }

    void Write(const std::string& path, const std::string& content) {

        //std::setlocale(LC_ALL, "");
        //std::locale::global(std::locale(""));
        //try
        //{
        //    out << GeneralUtilities::Convert(path) << "\t\t" << GeneralUtilities::Convert(content) << std::endl;
        //    out.flush();
        //}
        //catch (std::exception& e) {
        //    std::cout << e.what() << std::endl;
        //}
    }

    void Write(const QString& path, const QString& content) {

        //std::setlocale(LC_ALL, "");
        //std::locale::global(std::locale(""));
        //try
        //{
        //    out << path << "\t\t" << content << std::endl;
        //    out.flush();
        //}
        //catch (std::exception& e) {
        //    std::cout << e.what() << std::endl;
        //}
    }

    void Write(const std::string& path, const QString& content) {

        stream << QString::fromStdString(path) << Qt::endl;
        stream << content << Qt::endl;
        //std::setlocale(LC_ALL, "");
        //std::locale::global(std::locale(""));
        //try
        //{
        //    auto p = GeneralUtilities::Convert(path);
        //    //out << p << "\t\t" << content << std::endl;
        //    out.write(p.c_str(), p.size());
        //    out.write(L"\t\t", 2);
        //    out.write(content.c_str(), content.size());
        //    out.flush();
        //}
        //catch (std::exception& e) {
        //    std::cout << e.what() << std::endl;
        //}
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
    for (auto it : config->GetPaths()) {

        //fs.AddPath(it.first);
        server.RegisterPath(it.first, it.second);

        {
            auto json = JsonNode::Create(logger);
            json->Add(KeyWords(Keys::NodeType), static_cast<int>(Tags::Path_Update));
            json->Add(TagWords(Tags::Path_Update), it.first);
            message_queue->Add(json->ToString());
        }
    }

    fs.Start();

    auto tcp_server = TcpServerFactory::CreateTcpServer(message_queue);
    tcp_server->ListenOn(0);

    try {
        server.Start();
    }
    catch(...){

    }

#ifdef TEST
    writer->close();
#endif

    fs.Stop();
    logger->Stop();

    return 1;
}
