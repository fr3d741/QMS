#pragma once

#include <Model/CommonMedia.h>
#include <CommonDefines.h>

#include <map>

namespace Media {

    class TvShow : public CommonMedia
    {
    public:
        TvShow(Logging::ILogger::Ptr logger, QFileInfo  entry);

        bool Init() override;
        std::map<QString, XmlNode> CreateXml() override;
        QString GetPath() const override;

    protected:
        const char* Tmdb(TmdbTags key) override;
        QString GetFileName() override;
        QString GetDetails(QString id) override;
        void CreateEpisodeNfos(std::map<QString, XmlNode>& dir_entry);
        void extractImages(XmlNode& root, JsonNode::Ptr json, std::function<const char* (TmdbTags key)> tmdb) override;
    private:
        QFileInfo _episode_entry; // set when only a single episode needs to be scanned

        bool initCore(QString result_in_json);
        void CreateSingleEpisodeNfo(std::map<QString, XmlNode>& result_map);
        XmlNode CreateEpisodeXml(JsonNode::Ptr json, const std::string& episode_nr);
    };

}
