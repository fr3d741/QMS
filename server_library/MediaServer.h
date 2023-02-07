#pragma once

#include "Macros.h"
#include <Utility/IMessageQueue.h>
#include <Logging/ILogger.h>
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
    std::map<QString, int> _path_types;
    IStreamWriter::Ptr _writer;
    bool _continue = true;
    bool _dirty_cache = false;

public:
    MediaServer(Logging::ILogger::Ptr logger, IMessageQueue::Ptr queue, IStreamWriter::Ptr writer);

    void Start();
    void RegisterPath(const QString& path, int type);

private:
    void processMessage(QString& message);
    void updatePath(QFileInfo dir_entry, MediaType media_type);
    void loadCache();
    bool saveCache();
};
