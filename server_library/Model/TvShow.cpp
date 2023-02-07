#include <Model/TvShow.h>
#include <Networking/RestApi.h>
#include <Configuration/IConfiguration.h>
#include <Utility/GeneralUtilities.h>

#include <regex>
#include <algorithm>
#include <string>
#include <QFileInfo>
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

using namespace Media;
using G = GeneralUtilities;

static XmlNode
CreateEpisodeXml(JsonNode::Ptr json) {

    XmlNode root("episodedetails");
    root.AddChild(KodiWords(KodiTags::title), json->GetString(TmdbWords(TmdbTags::name, MediaType::TvShow)));
    root.AddChild(KodiWords(KodiTags::season), QString::number(json->GetInt(TmdbWords(TmdbTags::season_number, MediaType::TvShow))));
    root.AddChild(KodiWords(KodiTags::episode), QString::number(json->GetInt(TmdbWords(TmdbTags::episode_number, MediaType::TvShow))));
    root.AddChild(KodiWords(KodiTags::plot), json->GetString(TmdbWords(TmdbTags::overview, MediaType::TvShow)));

    return root;
}

static std::string 
Format(int season, int episode) {

    std::string season_str;
    std::string episode_str;
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(3) << season;
        season_str = stream.str();
    }
    {
        std::stringstream stream;
        stream << std::setfill('0') << std::setw(3) << episode;
        episode_str = stream.str();
    }
    return "S" + season_str + "E" + episode_str;
}

static std::string
ConvertToCommonFormat(const QString& str) {

    const std::regex number_filter("[0-9]{1,3}");
    const std::string std_str = str.toStdString();
    auto it = std::sregex_iterator(std_str.begin(), std_str.end(), number_filter);

    auto season = stoi(it->str());
    ++it;
    auto episode = stoi(it->str());
    return Format(season, episode);
}

static void
GatherEpisodesFromFS(Logging::ILogger::Ptr logger, QDir dir_entry, std::map<std::string, QString>& season_episodes) {

    std::setlocale(LC_ALL, "");
    std::locale::global(std::locale(""));

    for (auto const& season_dir : dir_entry.entryInfoList(QDir::NoDotAndDotDot | QDir::AllDirs)) {

        const std::regex season_filter("season.[0-9]{1,3}", std::regex_constants::icase);
        std::smatch match;

        auto dir_name = season_dir.fileName().toStdString();
        if (std::regex_search(dir_name, match, season_filter) == false)
            continue;

        QDir season(season_dir.absoluteFilePath());
        for (auto const& file_entry : season.entryInfoList(QDir::NoDotAndDotDot | QDir::Files) ) {
            // Looking for SxxExx pattern to identify episodes

            if (CommonMedia::IsMediaFile(file_entry.completeSuffix()) == false)
                continue;

            const static QRegularExpression episode_filter("S[0-9]{1,3}E[0-9]{1,3}", QRegularExpression::PatternOption::CaseInsensitiveOption);

            auto episode_match = episode_filter.match(file_entry.fileName());
            if (episode_match.hasMatch() == false)
            {
                XmlNode root("Incorrect file pattern");
                root.AddAttribute("file", file_entry.absoluteFilePath());
                logger->DebugMsg(root.Dump());
                continue;
            }

            season_episodes.insert({ ConvertToCommonFormat(episode_match.captured()), file_entry.absoluteFilePath() });
        }
    }
}

static void
GatherEpisodesFromJson(JsonNode::Ptr json, std::map<std::string, JsonNode::Ptr>& episodes_map) {

    auto episodes_json = json->GetArray(TmdbWords(TmdbTags::episodes, MediaType::TvShow));
    auto season_nr = -1;

    if (json->Has(TmdbWords(TmdbTags::order, MediaType::TvShow)))
        season_nr = json->GetInt(TmdbWords(TmdbTags::order, MediaType::TvShow));
    else if (json->Has(TmdbWords(TmdbTags::season_number, MediaType::TvShow)))
        season_nr = json->GetInt(TmdbWords(TmdbTags::season_number, MediaType::TvShow));
    else
        return;

    for (size_t i = 0; i < episodes_json.size(); ++i) {

        const std::string id = Format(season_nr, i+1);
        episodes_map.insert({ id, episodes_json[i] });
    }
}

static bool 
operator<(const std::pair<const std::string, JsonNode::Ptr>& a, const std::pair<const std::string, QString>& b) {
    return a.first < b.first;
}

static bool 
operator<(const std::pair<const std::string, QString>& a, const std::pair<const std::string, JsonNode::Ptr>& b) {
    return a.first < b.first;
}

using InputIt1 = std::map<std::string, JsonNode::Ptr>::iterator;
using InputIt2 = std::map<std::string, QString>::const_iterator;

static bool 
is_includes(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2) {//custom specialization of std::includes
    for (; first2 != last2; ++first1)
    {
        if (first1 == last1 || *first2 < *first1)
            return false;
        if (!(*first1 < *first2))
            ++first2;
    }
    return true;
}

static int 
count_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2)
{
    int counter = 0;
    while (first1 != last1 && first2 != last2)
    {
        if (*first1 < *first2)
            ++first1;
        else
        {
            if (!(*first2 < *first1))
                ++counter; // *first1 and *first2 are equivalent.
            ++first2;
        }
    }
    return counter;
}

static std::map<std::string, JsonNode::Ptr>
GetAllSeasonConfigurations(Logging::ILogger::Ptr logger, const int nr_of_seasons, const QString& show_id, const std::map<std::string, QString>& reference) {

    std::map<std::string, JsonNode::Ptr> to_return;
    int ret_count = 0;
    {
        std::map<std::string, JsonNode::Ptr> episodes_map;
        // Seasons stored with TvShow details
        for (auto i = 0; i <= nr_of_seasons; ++i) {
            auto response_json = JsonNode::Parse(logger, RestApi::Season(show_id, i, logger));
            if (response_json->Has(TmdbWords(TmdbTags::episodes, MediaType::TvShow)) == false)
                continue;
            GatherEpisodesFromJson(response_json, episodes_map);
        }

        if (is_includes(episodes_map.begin(), episodes_map.end(), reference.begin(), reference.end()))
            return episodes_map;

        int count = count_intersection(episodes_map.begin(), episodes_map.end(), reference.begin(), reference.end());
        if (ret_count < count) {
            to_return = std::move(episodes_map);
        }
    }

    {
        // check for alternate episode groups
        auto episode_groups_json_string = RestApi::EpisodeGroups(show_id, logger);
        auto ep_group_json = JsonNode::Parse(logger, episode_groups_json_string);
        auto group_results_array = ep_group_json->GetArray(TmdbWords(TmdbTags::results, MediaType::TvShow));
        if (group_results_array.empty())
            return to_return;

        for (auto it : group_results_array) {
            auto group_id = it->GetString(TmdbWords(TmdbTags::id, MediaType::TvShow));
            auto group_str = RestApi::EpisodeGroup(group_id, logger);
            auto group_json = JsonNode::Parse(logger, group_str);

            std::map<std::string, JsonNode::Ptr> episodes_map;
            auto group_array = group_json->GetArray(TmdbWords(TmdbTags::groups, MediaType::TvShow));
            for (size_t i = 0; i < group_array.size(); ++i) {

                GatherEpisodesFromJson(group_array[i], episodes_map);
            }

            if (is_includes(episodes_map.begin(), episodes_map.end(), reference.begin(), reference.end()))
                return episodes_map;

            auto count = count_intersection(episodes_map.begin(), episodes_map.end(), reference.begin(), reference.end());
            if (ret_count < count) {
                to_return = std::move(episodes_map);
            }
        }
    }

    return to_return;
}

TvShow::TvShow(Logging::ILogger::Ptr logger, QFileInfo entry)
    : CommonMedia(logger, entry, MediaType::TvShow) {
}

bool
TvShow::Init() {

    auto result_in_json = RestApi::SearchTv(_title, _logger);
    if (initCore(result_in_json))
        return true;

    _logger->DebugMsg(QString("Searching without year for %1 => %2").arg(_title).arg(_entry.absoluteFilePath()));
    result_in_json = RestApi::SearchTvWithoutYear(_title, _logger);
    if (initCore(result_in_json))
        return true;

    _logger->DebugMsg(QString("Searching by replaceing character for %1 => %2").arg(_title).arg(_entry.absoluteFilePath()));
    QString new_title;
    for(auto c : _title) {
        if (c.isDigit() || c.isLetter() || c == '(' || c == ')')
            new_title.append(c);
        else
            new_title.append(' ');
    }

    result_in_json = RestApi::SearchTvWithoutYear(new_title, _logger);
    if (initCore(result_in_json))
        return true;

    return false;
}

std::map<QString, XmlNode>
TvShow::CreateXml() {

    auto result_map = CommonMedia::CreateXml();
    CreateEpisodeNfos(result_map);
    return result_map;
}

const char* 
TvShow::Tmdb(TmdbTags key) {
    return TmdbWords(key, MediaType::TvShow);
}

QString
TvShow::GetFileName() {

    return _entry.absoluteFilePath().append("\\tvshow.nfo");
}

QString
TvShow::GetDetails(QString id) {
    return RestApi::TvDetails(id, _logger);
}

void
TvShow::CreateEpisodeNfos(std::map<QString, XmlNode>& result_map) {

    std::map<std::string, QString> season_episodes;
    QDir dir(_entry.absoluteFilePath());
    GatherEpisodesFromFS(_logger, dir, season_episodes);

    auto show_id = _details->GetInt(Tmdb(TmdbTags::id));
    auto seasons = _details->GetArray(Tmdb(TmdbTags::seasons));
    auto nr_of_seasons = seasons.size();
    auto proposed_episode_mapping = GetAllSeasonConfigurations(_logger, static_cast<int>(nr_of_seasons), QString::number(show_id), season_episodes);

    const std::filesystem::path extension(".nfo");
    for (auto&& item : season_episodes) {

        if (proposed_episode_mapping.count(item.first) < 1)
            continue;

        auto epiosde_json = proposed_episode_mapping[item.first];

        QFileInfo info(item.second);

        auto path = info.absolutePath() + "/" + info.completeBaseName().append(".nfo");
        result_map[path] = CreateEpisodeXml(epiosde_json);
    }
}

bool
TvShow::initCore(QString result_in_json) {

    if (result_in_json.isEmpty())
    {
        auto msg = "Empty request response for " + _title;
        _logger->DebugMsg(msg);
        return false;
    }

    auto json = JsonNode::Parse(_logger, result_in_json);
    _details = CommonMedia::GetDetails(json, [&](JsonNode::Ptr pp) { return pp->GetString(Tmdb(TmdbTags::name)); });
    if (_details == nullptr) {
        XmlNode root("tvshow");
        root.AddAttribute("error", "Cannot find details for " + _title);
        root.AddAttribute("path", _entry.absoluteFilePath());
        root.AddChild("json", result_in_json);
        _logger->DebugMsg(root.Dump());
        return false;
    }

    return true;
}

