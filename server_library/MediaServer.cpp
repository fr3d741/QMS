#include <MediaServer.h>
#include <CommonDefines.h>
#include <Utility/JsonNode.h>
#include <Model/CommonMedia.h>
#include <Model/TvShow.h>
#include <Utility/GeneralUtilities.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>

#include <QMultiMap>
#include <QList>
#include <QString>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

using namespace std::chrono_literals;
using G = GeneralUtilities;

static QMultiMap<int, QString>
MergeMessages(std::list<QString>& local_storage, Logging::ILogger::Ptr logger) {

    QMultiMap<int, QString> messages;
    for (auto msg : local_storage) {

        auto root = JsonNode::Parse(logger, msg);
        if (root->Has(KeyWords(Keys::NodeType)) == false)
        {
            logger->LogMessage("Invalid node type: " + msg);
            continue;
        }

        int type = root->GetInt(KeyWords(Keys::NodeType));
        if (type < 0 || static_cast<int>(Tags::Last) <= type)
        {
            logger->LogMessage("Invalid node type: " + msg);
            continue;
        }

        if (messages.contains(type))
        {
            auto it_pair = messages.equal_range(type);
            bool is_found = false;
            for (auto it = it_pair.first; it != it_pair.second && !is_found; ++it) {
                if (it.value().compare(msg) == 0)
                    is_found = true;
            }

            if (!is_found)
                messages.insert(type, msg );
        }
        else
            messages.insert(type, msg);
    }

    return messages;
}

MediaServer::MediaServer(Logging::ILogger::Ptr logger, IMessageQueue::Ptr queue, IStreamWriter::Ptr writer)
    : _logger(logger)
    , _queue(queue)
    , _writer(writer) {

}

void 
MediaServer::Start()
{
    std::list<QString> local_storage;
    std::chrono::steady_clock::time_point clock;
    bool count_started = false;
    bool do_continue = true;
    while (do_continue) {

        if (_queue->HasMessage()) {
            auto msg = _queue->BulkPop();
            while (msg.empty() == false) {
                local_storage.push_back(msg.front());
                msg.pop();
            }
            clock = std::chrono::steady_clock::now();
            count_started = true;
        }

        if (count_started) {

            auto now = std::chrono::steady_clock::now();
            if (9s <= (now - clock)) {

                count_started = false;

                auto messages = MergeMessages(local_storage, _logger);
                local_storage.clear();

                for (auto item : messages) {
                    processMessage(item);
                }
                std::cout << "finished " <<std::endl;
                break;
            }
        }

        std::this_thread::sleep_for(100ms);
    }
}

void
MediaServer::RegisterPath(const QString& path, int type) {
    _path_types[path] = type;
}

void 
MediaServer::processMessage(QString& message) {

    auto root = JsonNode::Parse(_logger, message);
    if (root->Has(KeyWords(Keys::NodeType)) == false)
    {
        _logger->LogMessage("Invalid node type: " + message);
        return;
    }

    int type = root->GetInt(KeyWords(Keys::NodeType));
    if (type < 0 || static_cast<int>(Tags::Last) <= type)
    {
        _logger->LogMessage("Invalid node type: " + message);
        return;
    }

    switch (static_cast<Tags>(type)) {
        case Tags::Path_Update: {
            QString path = root->GetString(TagWords(Tags::Path_Update));
            auto media_type = (MediaType)_path_types[path];

            QDir dir(path);
            for (auto const& dir_entry : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs)) {

                auto path = dir_entry.absoluteFilePath();
                { // do not parse until cache cleared
                    
                    if (_cache.contains(path))
                        continue;
                    _cache.insert(path);
                }

                qDebug() << dir_entry;
                std::shared_ptr<Media::CommonMedia> media;
                switch (media_type)
                {
                    case Movie:
                        media = std::make_shared<Media::CommonMedia>(_logger, dir_entry);
                        break;
                    case TvShow:
                        media = std::make_shared<Media::TvShow>(_logger, dir_entry);
                        break;
                    default:
                        break;
                }

                std::cout << "___" << __FILE__ << "::" << __LINE__ << std::endl;
                if (media->Init() == false)
                    continue;

                auto nodes = media->CreateXml();

                for (auto&& it : nodes) {

                    _writer->Write(it.first, it.second.Dump());
                }
            }
        }
        break;
        case Tags::Last:
        default:
            break;
    }

}
