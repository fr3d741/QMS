#include <Networking/RestApi.h>
#include <Utility/ScopedFunction.h>
#include <Utility/GeneralUtilities.h>

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
SendRequest(const QString& request_str) {

    { 
//        // Implement rate limiting: 20 req/s
//        // According to TMDB API there is no such limit, but in practice there is
//        static int request_count = 0;
//        static std::chrono::time_point last_time = std::chrono::steady_clock::now();
//        static long long last_second = 0;

//        request_count++;

//        const std::chrono::time_point now = std::chrono::steady_clock::now();
//        const std::chrono::duration duration = now - last_time;
//        const long long second = std::chrono::duration_cast<std::chrono::seconds>(duration).count();
//        if (last_second != second) {
//            request_count = 0;
//            last_second = second;
//            last_time = now;
//        }
//        else if (duration < 1s && 20 <= request_count) {
//            std::this_thread::sleep_for(1s - duration);
//        }
    }

#ifndef  DISABLE_REST_API

    std::string std_request = request_str.toStdString();
    std::string result;
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
        return "";

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
        //TODO: Log
        return "";
    }

    return QString::fromStdString(result);
}

static QString
CreateSearchRequest(const QString title, const char* pattern,  const QString& api_key) {

    static QRegularExpression regex("[(][0-9]{4}[)]");

    auto match = regex.match(title);
    if (match.hasMatch() == false){
        // TODO: log
        return "";
    }

    auto title_wo_year = title.mid(0, match.capturedStart()).trimmed();
    auto byte_array = QUrl::toPercentEncoding(title_wo_year);
    auto intermediate = match.captured();
    auto year = G::ConvertToYear(intermediate);

    return QString(pattern).arg(api_key).arg(byte_array).arg(year);
}

QString& RestApi::ApiKey() {
    static QString api_key;
    return api_key;
}

QString
RestApi::SearchMovie(const QString& title){

    auto request = CreateSearchRequest(title, "https://api.themoviedb.org/3/search/movie?api_key=%1&query=%2&page=1&include_adult=false&year=%3", ApiKey());
    return SendRequest(request);
}

QString
RestApi::SearchTv(const QString& title){

    auto request = CreateSearchRequest(title, "https://api.themoviedb.org/3/search/tv?api_key=%1&page=1&query=%2&include_adult=false&first_air_date_year=%3", ApiKey());
    return SendRequest(request);
}

QString
RestApi::MovieDetails(const QString& id) {

    auto request = "https://api.themoviedb.org/3/movie/" + id + "?api_key=" + ApiKey() + "&append_to_response=images,translations,keywords&include_image_language=en,hu,null";
    return SendRequest(request);
}

QString
RestApi::TvDetails(const QString& id) {

    auto request = "https://api.themoviedb.org/3/tv/" + id + "?api_key=" + ApiKey() + "&append_to_response=images,translations,keywords,credits&include_image_language=en,HU,null";
    return SendRequest(request);
}

QString
RestApi::EpisodeGroups(const QString& id) {

    auto request = "https://api.themoviedb.org/3/tv/" + id + "/episode_groups?api_key=" + ApiKey();
    return SendRequest(request);
}

QString
RestApi::EpisodeGroup(const QString& group_id) {

    auto request = "https://api.themoviedb.org/3/tv/episode_group/" + group_id + "?api_key=" + ApiKey() + "&append_to_response=credits";
    return SendRequest(request);
}

QString
RestApi::Season(const QString& id, int season_nr) {

    auto request = "https://api.themoviedb.org/3/tv/" + id + "/season/" + QString::number(season_nr) + "?api_key=" + ApiKey() + "&append_to_response=translations";
    return SendRequest(request);
}

