#include <Networking/RestApi.h>
#include <Utility/ScopedFunction.h>
#include <Utility/GeneralUtilities.h>

#include <Configuration/IConfiguration.h>

#include <QUrl>
#include <QRegularExpression>

#include <curl/curl.h>

#include <regex>
#include <array>
#include <chrono>
#include <iostream>
#include <assert.h>
#include <thread>

//#define DISABLE_REST_API

using namespace std::chrono_literals;
using G = GeneralUtilities;

size_t 
write_callback(char* ptr, size_t /*size*/, size_t nmemb, void* userdata) {

    auto str = reinterpret_cast<std::string*>(userdata);
    str->append(ptr, nmemb);
    return nmemb;
}

static QString
SendRequest(const QString& request_str, Logging::ILogger::Ptr log, const QString& api_key) {

    { 
        // Implement rate limiting: 20 req/s
        // According to TMDB API there is no such limit, but in practice there is
        static int request_count = 0;
        static std::chrono::time_point last_time = std::chrono::steady_clock::now();
        static long long last_second = 0;

        request_count++;

        const std::chrono::time_point now = std::chrono::steady_clock::now();
        const std::chrono::duration duration = now - last_time;
        const long long second = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
        if (last_second != second) {
            request_count = 0;
            last_second = second;
            last_time = now;
        }
        else if (duration < 1s && 20 <= request_count) {
            std::this_thread::sleep_for(1s - duration);
        }
    }

#ifndef  DISABLE_REST_API

    std::string std_request = QString(request_str).arg(api_key).toStdString();
    std::string result;
    CURL* curl = curl_easy_init();
    if (curl == nullptr){
        log->Error(QString("CURL init failed"));
        return "";
    }

    ScopedFunction fn_curl([=]() { curl_easy_cleanup(curl); });
    CURLcode res;
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_URL, std_request.data());
    res = curl_easy_perform(curl);
#endif //  DISABLE_REST_API

    if (res != CURLE_OK) {
        log->DebugMsg(QString("Error: %1, Request: %2").arg(res).arg(request_str));
        return "";
    }

    log->LogMessage(request_str, Logging::LogLevel::All);

    return QString::fromStdString(result);
}

static QString
GetUriEncodedTitleWithoutYear(const QString title){

    static QRegularExpression regex("[(][0-9]{4}[)]");

    QString title_wo_year;
    auto match = regex.match(title);
    if (match.hasMatch() == false)
        title_wo_year = title;
    else
        title_wo_year = title.mid(0, match.capturedStart()).trimmed();

    auto byte_array = QUrl::toPercentEncoding(title_wo_year);
    return QString(byte_array);
}



QString& RestApi::ApiKey() {
    static QString api_key;
    return api_key;
}

QString
RestApi::SearchMovie(const QString& title, Logging::ILogger::Ptr log){

    auto encoded = GetUriEncodedTitleWithoutYear(title);
    int year;
    QString request = "https://api.themoviedb.org/3/search/movie?api_key=%1&query=" + encoded + "&page=1&include_adult=false";
    if (G::GetYear(title, year))
        request += "&year=" + QString::number(year);
    return SendRequest(request, log, ApiKey());
}

QString
RestApi::SearchMovieWithoutYear(const QString& title, Logging::ILogger::Ptr log){

    auto encoded = GetUriEncodedTitleWithoutYear(title);
    QString request = "https://api.themoviedb.org/3/search/movie?api_key=%1&query=" + encoded + "&page=1&include_adult=false";
    return SendRequest(request, log, ApiKey());
}

QString
RestApi::SearchTvWithoutYear(const QString& title, Logging::ILogger::Ptr log){

    auto encoded = GetUriEncodedTitleWithoutYear(title);
    QString request = "https://api.themoviedb.org/3/search/tv?api_key=%1&query=" + encoded + "&page=1&include_adult=false";
    return SendRequest(request, log, ApiKey());
}

QString
RestApi::SearchTv(const QString& title, Logging::ILogger::Ptr log){

    auto encoded = GetUriEncodedTitleWithoutYear(title);
    int year;
    QString request = "https://api.themoviedb.org/3/search/tv?api_key=%1&query=" + encoded + "&page=1&include_adult=false";
    if (G::GetYear(title, year))
        request += "&first_air_date_year=" + QString::number(year);

    return SendRequest(request, log, ApiKey());
}

QString
RestApi::MovieDetails(const QString& id, Logging::ILogger::Ptr log) {

    auto request = "https://api.themoviedb.org/3/movie/" + id + "?api_key=%1&append_to_response=images,translations,keywords,credits&include_image_language=en,hu,null";
    return SendRequest(request, log, ApiKey());
}

QString
RestApi::TvDetails(const QString& id, Logging::ILogger::Ptr log) {

    auto request = "https://api.themoviedb.org/3/tv/" + id + "?api_key=%1&append_to_response=images,translations,keywords,credits&include_image_language=en,HU,null";
    return SendRequest(request, log, ApiKey());
}

QString
RestApi::EpisodeGroups(const QString& id, Logging::ILogger::Ptr log) {

    auto request = "https://api.themoviedb.org/3/tv/" + id + "/episode_groups?api_key=%1";
    return SendRequest(request, log, ApiKey());
}

QString
RestApi::EpisodeGroup(const QString& group_id, Logging::ILogger::Ptr log) {

    auto request = "https://api.themoviedb.org/3/tv/episode_group/" + group_id + "?api_key=%1&append_to_response=credits";
    return SendRequest(request, log, ApiKey());
}

QString
RestApi::Season(const QString& id, int season_nr, Logging::ILogger::Ptr log) {

    auto request = "https://api.themoviedb.org/3/tv/" + id + "/season/" + QString::number(season_nr) + "?api_key=%1&append_to_response=translations";
    return SendRequest(request, log, ApiKey());
}

