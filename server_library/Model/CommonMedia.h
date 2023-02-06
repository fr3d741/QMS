#pragma once

#include <CommonDefines.h>
#include <Utility/XmlNode.h>
#include <Utility/JsonNode.h>
#include <Logging/ILogger.h>

#include <filesystem>
#include <functional>
#include <QFileInfo>

namespace Media {

    class CommonMedia
    {
    protected:
        Logging::ILogger::Ptr _logger;
        const QString _title;
        const QFileInfo _entry;
        ::MediaType _media_type;
        JsonNode::Ptr _details;
    public:
        CommonMedia(Logging::ILogger::Ptr logger, QFileInfo entry, ::MediaType media_type = ::MediaType::Movie);

        enum Property {
            title,
            originaltitle,
            year,
            ratings,
            userrating,
            set,
            plot,
            outline,
            tagline,
            runtime,
            poster,
            fanart,
            mpaa,
            certification,
            id,
            country,
            premiered,
            genre,
            studio,
            credits,
            tag,
            actor,
            languages,
            dateadded,
        };

        virtual void Fill(Logging::ILogger::Ptr logger, XmlNode& root, JsonNode::Ptr json, const std::vector<Property>& properties);
        virtual bool Init();
        virtual std::map<QString, XmlNode> CreateXml();

        static bool IsMediaFile(const QString& path);

    protected:
        virtual const char* Tmdb(TmdbTags key);
        virtual QString GetDetails(QString id);
        virtual QString GetFileName();
        virtual JsonNode::Ptr GetDetails(JsonNode::Ptr json_ptr, std::function<QString (JsonNode::Ptr)> get_title_fn);
    private:
        bool initCore(QString result_in_json);
    };
}
