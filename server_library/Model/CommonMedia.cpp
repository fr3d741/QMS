#include <Model/CommonMedia.h>
#include <CommonDefines.h>
#include <Networking/RestApi.h>
#include <Utility/GeneralUtilities.h>
#include <Configuration/IConfiguration.h>

#include <regex>
#include <set>
#include <QDir>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

using namespace Media;
using G = GeneralUtilities;

struct Key {
    const QString _hash;

    QString title;
    int year;
    mutable int hamming_distance;

    Key(const QString& t, const int y)
        :_hash(QString("%1%2").arg(t).arg(y)), title(t), year(y) {
    }

    bool operator<(const Key& other) const {
        return _hash < other._hash;
    }
};

struct KeyComp {

};

static int 
GetReleaseYear(JsonNode::Ptr ptr, MediaType media_type) {

    auto date = ptr->GetString(::TmdbWords(TmdbTags::release_date, media_type));
    return G::ConvertToYear(date);
}

static JsonNode::Ptr
SelectOne(Logging::ILogger::Ptr logger, MediaType media_type, const QString& expected_title, const std::vector<JsonNode::Ptr>& values, std::function<QString(JsonNode::Ptr)> get_title_fn, bool& is_match_date, bool& is_match_title) {

    std::map<Key, JsonNode::Ptr> result_mapping;
    const QRegularExpression regex("[(][0-9]{4}[)]");
    auto match = regex.match(expected_title);
    QString title_wo_year = expected_title.mid(0, match.capturedStart());
    int release_year = G::ConvertToYear(match.captured());
    title_wo_year = title_wo_year.trimmed();
    
    JsonNode::Ptr chosen = values[0];
    is_match_date = false;
    is_match_title = false;

    for (size_t i = 0; i < values.size(); ++i) {

        auto ptr = values[i];
        const int local_year = GetReleaseYear(ptr, media_type);
        auto title = get_title_fn(ptr);

        is_match_date = local_year == release_year;
        is_match_title = title.compare(title_wo_year, Qt::CaseInsensitive) == 0;
        if (!is_match_title) {
            if (G::HasNumber(title)) {

                auto title_with_roman_number = G::ConvertLatinToRomanNumbers(title);
                is_match_title = title_with_roman_number.compare(title_wo_year, Qt::CaseInsensitive) == 0;
                if (is_match_title)
                    title = title_with_roman_number;
            }
            else if (G::HasNumber(title_wo_year)) {

                auto title_with_roman_number = G::ConvertLatinToRomanNumbers(title_wo_year);
                is_match_title = title.compare(title_with_roman_number,Qt::CaseInsensitive) == 0;
                if (is_match_title)
                    title_wo_year = title_with_roman_number;
            }
        }

        if (is_match_date && is_match_title) {
            chosen = ptr;
            break;
        }
        else {
            result_mapping.insert({ Key{ title, local_year }, ptr });
        }
    }

    if (!is_match_date || !is_match_title) {
        std::for_each(result_mapping.begin(), result_mapping.end(), [&](std::pair<const Key, JsonNode::Ptr> item) { item.first.hamming_distance = G::HammingDistance(item.first.title, title_wo_year); });
        auto it = std::min_element(result_mapping.begin(), result_mapping.end(), [](std::pair<const Key, JsonNode::Ptr>& a, std::pair<const Key, JsonNode::Ptr>& b) { return a.first.hamming_distance < b.first.hamming_distance; });
        if (it != result_mapping.end()) {
            chosen = it->second;
        }
    }

    if (is_match_date && is_match_title) {
        if (IConfiguration::Instance().IsDebug()) {
            XmlNode root("Multiple results");
            root.AddAttribute("title", expected_title);

            for (auto item : result_mapping)
            {
                root.AddChild("Element", item.first._hash);
            }
            logger->LogMessage(root.Dump());
        }
    }

    return chosen;
}

static void
extractImages(XmlNode& root, JsonNode::Ptr json, std::function<const char* (TmdbTags key)> tmdb) {

    if (json->Has(tmdb(TmdbTags::images)) == false)
        return;

    auto object = json->GetPointer(tmdb(TmdbTags::images));
    if (object == nullptr)
        return;

    if (object->Has(tmdb(TmdbTags::posters)) == false)
        return;

    auto array = object->GetArray(tmdb(TmdbTags::posters));
    for (auto&& item : array) {
        if (item->Has(tmdb(TmdbTags::iso_639_1)) == false)
            continue;

        auto lang = item->GetString(tmdb(TmdbTags::iso_639_1));
        if (lang == "hu" || lang == "en") {
            auto path = item->GetString(tmdb(TmdbTags::file_path));
            QString link = "https://image.tmdb.org/t/p/original" + path;
            XmlNode& child = root.AddChild(KodiWords(KodiTags::thumb), link);
            child.AddAttribute(KodiWords(KodiTags::aspect), KodiWords(KodiTags::poster));
            break;
        }
    }
}

static JsonNode::Ptr
extractCountryLocal(Logging::ILogger::Ptr logger, JsonNode::Ptr json, const char* country_locale, std::function<const char*(TmdbTags key)> fn) {

    if (json->Has(fn(TmdbTags::translations)) == false)
        return JsonNode::Create(logger); // empty node

    auto ptr = json->GetPointer(fn(TmdbTags::translations));
    if (ptr->Has(fn(TmdbTags::translations)) == false)
        return JsonNode::Create(logger); // empty node

    auto array = ptr->GetArray(fn(TmdbTags::translations));
    for (auto array_ptr : array) {

        if (array_ptr->Has(fn(TmdbTags::iso_639_1)) == false)
            continue;

        auto lang = array_ptr->GetString(fn(TmdbTags::iso_639_1));
        if (lang.compare(country_locale) != 0)
            continue;

        return array_ptr->GetPointer(fn(TmdbTags::data));
    }

    return JsonNode::Create(logger); // empty node
}

static void
extractActors(XmlNode& root, JsonNode::Ptr json, std::function<const char* (TmdbTags key)> tmdb) {

    if (json->Has(tmdb(TmdbTags::credits)) == false)
        return;

    auto credits = json->GetPointer(tmdb(TmdbTags::credits));
    if (credits->Has(tmdb(TmdbTags::cast)) == false)
        return;

    auto actors = credits->GetArray(tmdb(TmdbTags::cast));
    for (auto actor_object : actors) {
        XmlNode& actor = root.AddChild(KodiWords(KodiTags::actor));

        actor.AddChild(KodiWords(KodiTags::name), actor_object->GetString(tmdb(TmdbTags::name)));
        actor.AddChild(KodiWords(KodiTags::role), actor_object->GetString(tmdb(TmdbTags::character)));

        auto img = actor_object->GetString(tmdb(TmdbTags::character));
        QString link = "https://image.tmdb.org/t/p/original";
        link.append(img);
        actor.AddChild(KodiWords(KodiTags::thumb), link);

        auto id = QString::number(actor_object->GetInt(tmdb(TmdbTags::id)));
        QString profile_link = "https://www.themoviedb.org/person/";
        profile_link.append(id);
        actor.AddChild(KodiWords(KodiTags::profile), profile_link);
        actor.AddChild(KodiWords(KodiTags::tmdbid), id);
    }
}

CommonMedia::CommonMedia(Logging::ILogger::Ptr logger, QFileInfo entry, MediaType media_type)
    : _logger(logger)
    , _title(entry.fileName())
    , _entry(entry)
    ,_media_type(media_type){
}

void
CommonMedia::Fill(Logging::ILogger::Ptr logger, XmlNode& root, JsonNode::Ptr json, const std::vector<Property>& properties) {

    auto country_local = extractCountryLocal(logger, json, "hu", [&](TmdbTags key) { return this->Tmdb(key); });
    for (auto prop : properties) {

        switch (prop)
        {
        case title:
            if (country_local->Has(Tmdb(TmdbTags::title)))
                root.AddChild(KodiWords(KodiTags::title), country_local->GetString(Tmdb(TmdbTags::title)));
            else
                root.AddChild(KodiWords(KodiTags::title), json->GetString(Tmdb(TmdbTags::original_title)));
            break;
        case originaltitle:
            root.AddChild(KodiWords(KodiTags::original_title), json->GetString(Tmdb(TmdbTags::original_title)));
            break;
        case year:
            if (json->Has(Tmdb(TmdbTags::release_date)))
            {
                auto date = json->GetString(Tmdb(TmdbTags::release_date));

//                const QRegularExpression regex("[0-9]{4}");
//                std::smatch match;
//                if (std::regex_search(date, match, regex) == false)
//                    break;

//                std::string year = match.str();
                auto date_time = QDate::fromString(date);
                root.AddChild(KodiWords(KodiTags::year), QString::number(date_time.year()));
                root.AddChild(KodiWords(KodiTags::premiered), date);
            }
            break;
        case ratings:
            break;
        case userrating:
            break;
        case set:
            if (json->Has(Tmdb(TmdbTags::belongs_to_collection))) {

                auto ptr = json->GetPointer(Tmdb(TmdbTags::belongs_to_collection));
                if (ptr == nullptr)
                    break;
                auto name = ptr->GetString(Tmdb(TmdbTags::name));
                XmlNode& set = root.AddChild(KodiWords(KodiTags::set));
                set.AddChild(KodiWords(KodiTags::name), name);
                set.AddChild(KodiWords(KodiTags::overview), "");
            }
            break;
        case plot:
            if (country_local->Has(Tmdb(TmdbTags::overview))) {
                auto plot = country_local->GetString(Tmdb(TmdbTags::overview));
                root.AddChild(KodiWords(KodiTags::plot), plot);
            }
            break;
        case outline:
            if (country_local->Has(Tmdb(TmdbTags::overview))) {
                auto plot = country_local->GetString(Tmdb(TmdbTags::overview));
                root.AddChild(KodiWords(KodiTags::outline), plot);
            }
            break;
        case tagline:
            break;
        case runtime:
            if (country_local->Has(Tmdb(TmdbTags::runtime))) {
                auto runtime = QString::number(country_local->GetInt(Tmdb(TmdbTags::runtime)));
                root.AddChild(KodiWords(KodiTags::runtime), runtime);
            }
            else if (json->Has(Tmdb(TmdbTags::runtime))) {
                auto runtime = QString::number(json->GetInt(Tmdb(TmdbTags::runtime)));
                root.AddChild(KodiWords(KodiTags::runtime), runtime);
            }
            break;
        case fanart:
        case poster:
            extractImages(root, json, [&](TmdbTags key) { return this->Tmdb(key); });
            break;
        case mpaa:
            break;
        case certification:
            break;
        case id:
            break;
        case country:
            break;
        case premiered:
            break;
        case genre:
            if (json->Has(Tmdb(TmdbTags::genres))) {

                auto array = json->GetArray(Tmdb(TmdbTags::genres));
                for (auto&& item : array) {
                    root.AddChild(KodiWords(KodiTags::genre), item->GetString("name"));
                }
            }
            break;
        case studio:
            break;
        case credits:
            break;
        case tag:
            if (json->Has(Tmdb(TmdbTags::keywords))) {
                auto object = json->GetPointer(Tmdb(TmdbTags::keywords));
                auto tags = object->FindTagRecursive(Tmdb(TmdbTags::name));
                for (auto item : tags) {

                    root.AddChild(KodiWords(KodiTags::tag), item);
                }

            }
            break;
        case actor:
            extractActors(root, json, [&](TmdbTags key) { return this->Tmdb(key); });
            break;
        case languages:
            break;
        case dateadded:
            break;
        default:
            break;
        }
    }
}

bool
CommonMedia::Init() {

    auto result_in_json = RestApi::SearchMovie(_title, _logger);
    if (result_in_json.isEmpty())
    {
        // TODO:
        _logger->LogMessage("Empty json for" + _title);
        return false;
    }
    auto json = JsonNode::Parse(_logger, result_in_json);
    _details = GetDetails(json, [&](JsonNode::Ptr pp) { return pp->GetString(Tmdb(TmdbTags::title)); });
    if (_details == nullptr) {
        XmlNode root("Init");
        auto msg = "Cannot find details for {}" + _title;
        root.AddAttribute("message", msg);
        //root.AddChild("CompleteJSON", json());
        _logger->LogMessage(root.Dump());
        return false;
    }

    return true;
}

JsonNode::Ptr
CommonMedia::GetDetails(JsonNode::Ptr json_ptr, std::function<QString(JsonNode::Ptr)> get_title_fn) {

    JsonNode& json = *json_ptr;
    if (json.Has("results") == false)
    {
        _logger->LogMessage("No results sectiton in " + json.ToString());
        return nullptr;
    }

    std::vector<JsonNode::Ptr> values;
    if (json.Get("results", values) == false || values.empty())
    {
        _logger->LogMessage("No results sectiton in " + json.ToString());
        return nullptr;
    }

    auto js = values[0];
    if (values.size() != 1) {

        bool is_match_date, is_match_title;
        js = SelectOne(_logger, _media_type, _title, values, get_title_fn, is_match_date, is_match_title);

        if (!is_match_date || !is_match_title) {
            XmlNode root("Multiple results");
            root.AddAttribute("title", _title);
            root.AddAttribute("path", _entry.absoluteFilePath());
            root.AddChild("Complete JSON", json.ToString());
            _logger->LogMessage(root.Dump());
        }
    }

    auto id = js->GetInt(Tmdb(TmdbTags::id));
    QString movie_json = GetDetails(QString::number(id));

    return JsonNode::Parse(_logger, movie_json);
}

const char* 
CommonMedia::Tmdb(TmdbTags key) {
    return TmdbWords(key, MediaType::Movie);
}

QString
CommonMedia::GetDetails(QString id) {
    return RestApi::MovieDetails(id, _logger);
}

QString
CommonMedia::GetFileName() {

    QString extension(".nfo");
    for (auto const& dir_entry : _entry.dir().entryInfoList(QDir::NoDotAndDotDot | QDir::Files) )
    {
        if (IsMediaFile(dir_entry.path()) == false)
            continue;

        auto path = dir_entry.absolutePath() + dir_entry.completeBaseName() + ".nfo";
        return path;
    }

    return _entry.absolutePath() + "\\movie.nfo";
}

std::map<QString, XmlNode>
CommonMedia::CreateXml() {

    std::map<QString, XmlNode> xmls;
    if (_details == nullptr)
    {
        _logger->LogMessage("This should never happen! No Json to create XML from!");
        return xmls;
    }

    XmlNode root = XmlNode(XmlTagFor(_media_type));
    XmlNode& id_node = root.AddChild(KodiWords(KodiTags::unique_id), QString::number(_details->GetInt(Tmdb(TmdbTags::id))));
    id_node.AddAttribute("type", "tmdb");
    id_node.AddAttribute("default", "true");
    if (_details->Has(Tmdb(TmdbTags::imdb_id))) {
        XmlNode& imdb_id_node = root.AddChild(KodiWords(KodiTags::unique_id), _details->GetString(Tmdb(TmdbTags::imdb_id)));
        imdb_id_node.AddAttribute("type", "imdb");
        imdb_id_node.AddAttribute("default", "false");
    }

    Fill(_logger, root, _details, { title,
                                originaltitle,
                                year,
                                set,
                                plot,
                                outline,
                                runtime,
                                poster,
                                genre,
                                credits,
                                tag,
                                actor });

    xmls[GetFileName()] = root;
    return xmls;
}

bool 
CommonMedia::IsMediaFile(const QString& path) {

    const std::set<QString> common_extensions = { ".mkv", ".avi", ".mpeg", ".mov", ".wmv", ".mp4", ".m4p", ".m4v" };
    for(auto ex : common_extensions){
        if (path.endsWith(ex))
            return true;
    }

    return false;
}
