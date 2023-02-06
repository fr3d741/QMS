#pragma once

#include <Model/CommonMedia.h>
#include <CommonDefines.h>

#include <filesystem>
#include <map>

namespace Media {

    class TvShow : public CommonMedia
    {
    public:
        TvShow(Logging::ILogger::Ptr logger, QFileInfo  entry);

        bool Init() override;
        std::map<QString, XmlNode> CreateXml() override;

    protected:
        const char* Tmdb(TmdbTags key) override;
        QString GetFileName() override;
        QString GetDetails(QString id) override;
        void CreateEpisodeNfos(std::map<QString, XmlNode>& dir_entry);
    private:
        bool initCore(QString result_in_json);
    };

}
