#pragma once

#include "Macros.h"
#include <Logging/ILogger.h>
#include <Networking/ITcpServer.h>
#include <Utility/IMessageQueue.h>
#include <Utility/StreamWriter.h>

#include <memory>
#include <string>
#include <set>
#include <map>

#include <QSet>
#include <QFileInfo>

class DLL MediaServer {

    Logging::ILogger::Ptr _logger;
    IMessageQueue::Ptr _queue;
    QSet<QString> _cache;
    IStreamWriter::Ptr _writer;
    Networking::ITcpServer::Ptr _server;
    bool _continue = true;
    bool _dirty_cache = false;

public:
    MediaServer(Logging::ILogger::Ptr logger, IMessageQueue::Ptr queue, IStreamWriter::Ptr writer, Networking::ITcpServer::Ptr server);

    void Start();

private:
    void processMessage(QString& message, const std::map<QString, int>& path_types);
    void updatePath(QFileInfo dir_entry, MediaType media_type);
    void loadCache();
    bool saveCache();
};
