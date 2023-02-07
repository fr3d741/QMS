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
            logger->Error("Invalid node type: " + msg);
            continue;
        }

        int type = root->GetInt(KeyWords(Keys::NodeType));
        if (type < 0 || static_cast<int>(Actions::Last) <= type)
        {
            logger->Error("Invalid node type: " + msg);
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

static int
GetMediaType(QString entry, const std::map<QString, int>& path_types){

    if (path_types.count(entry))
        return path_types.at(entry);

    QFileInfo info(entry);
    auto d = info.dir();
    if (d.isRoot())
        return -1;

    return GetMediaType(d.absolutePath(), path_types);
}

MediaServer::MediaServer(Logging::ILogger::Ptr logger, IMessageQueue::Ptr queue, IStreamWriter::Ptr writer)
    : _logger(logger)
    , _queue(queue)
    , _writer(writer) {

}

void 
MediaServer::Start() {

    std::list<QString> local_storage;
    std::chrono::steady_clock::time_point process_clock;
    std::chrono::steady_clock::time_point idle_clock;
    bool process_count_started = false;

    loadCache();

    while (_continue) {

        if (_queue->HasMessage()) {
            auto msg = _queue->BulkPop();
            while (msg.empty() == false) {
                local_storage.push_back(msg.front());
                msg.pop();
            }
            process_clock = std::chrono::steady_clock::now();
            process_count_started = true;
        }

        if (process_count_started) {

            auto now = std::chrono::steady_clock::now();
            if (9s <= (now - process_clock)) {

                process_count_started = false;

                auto messages = MergeMessages(local_storage, _logger);
                local_storage.clear();

                for (auto item : messages)
                    processMessage(item);

                idle_clock = std::chrono::steady_clock::now();
            }
        }

        if (_dirty_cache){
            auto now = std::chrono::steady_clock::now();
            if (9s <= (now - idle_clock)) {
                if (saveCache())
                    _dirty_cache = false;
                idle_clock = std::chrono::steady_clock::now();
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
        _logger->Error("Invalid node type: " + message);
        return;
    }

    int type = root->GetInt(KeyWords(Keys::NodeType));
    if (type < 0 || static_cast<int>(Actions::Last) <= type)
    {
        _logger->Error("Invalid node type: " + message);
        return;
    }

    switch (static_cast<Actions>(type)) {
        case Actions::Rescan:{
                QString path = root->GetString(KeyWords(Keys::Message));
                if (QFileInfo::exists(path)){
                    QFileInfo info(path);
                    auto type = GetMediaType(path, _path_types);
                    _cache.remove(path);
                    _dirty_cache = true;
                    updatePath(info, static_cast<MediaType>(type));
                }
            }
            break;
        case Actions::Path_Update: {
                QString path = root->GetString(KeyWords(Keys::Message));
                auto media_type = (MediaType)_path_types[path];

                QDir dir(path);
                for (auto const& dir_entry : dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs))
                    updatePath(dir_entry, media_type);
            }
            break;
        case Actions::Test:
            qDebug() << "Test message" << root->GetString(KeyWords(Keys::Message));
            break;
        case Actions::Quit:
            _continue = false;
            break;
        case Actions::Last:
        default:
            break;
    }
}

void
MediaServer::updatePath(QFileInfo dir_entry, MediaType media_type){

    auto path = dir_entry.absoluteFilePath();
    { // do not parse until cache cleared

        if (_cache.contains(path))
            return;
        _cache.insert(path);
        _dirty_cache = true;
    }

    qDebug() << path;

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

    if (media->Init() == false){
        _logger->Error("Failed to process " + path);
        return;
    }

    auto nodes = media->CreateXml();
    for (auto&& it : nodes) {

        _writer->Write(it.first, it.second.Dump());
    }
}

static bool
loadCache(QSet<QString>& cache){

    if (!QFileInfo::exists("cache"))
        return true;

    QFile file("cache");
    if (file.open(QIODevice::ReadOnly) == false)
        return false;

    cache.clear();

    QTextStream stream(&file);
    for(auto item : cache){
        cache.insert(stream.readLine());
    }
    file.close();
    return true;
}

void
MediaServer::loadCache(){

    const int max_retry = 30;
    int num_retry = 0;
    while(num_retry < max_retry && ::loadCache(_cache) == false){
        num_retry++;
        std::this_thread::sleep_for(1s);
    }

    if (num_retry == max_retry){
        _logger->Error("Failed to open cache file. Terminate");
        _continue = false;
    }
}

bool
MediaServer::saveCache(){

    QFile file("cache");
    if (file.open(QIODevice::WriteOnly) == false)
        return false;

    QTextStream stream(&file);
    for(auto&& item :  qAsConst(_cache)){
        stream << item << Qt::endl;
    }
    file.close();
    return true;
}
