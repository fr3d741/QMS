#pragma once

#include "Macros.h"
#include <array>
#include <regex>
#include <string>
#include <sstream>
#include <fstream>

#include <QString>

typedef wchar_t char_t;

enum class Keys {
    NodeType,
    Message,
    Last
};

enum class Actions {
    Path_Update,
    Test,
    Quit,
    Rescan,
    ReConfigure,
    Request,
    Ping,
    Last
};

enum MediaType {
    Movie = 0,
    TvShow = 1
};

enum class TmdbTags {
    original_title,
    id,
    imdb_id,
    genres,
    images,
    posters,
    iso_639_1,
    file_path,
    translations,
    data,
    title,
    release_date,
    belongs_to_collection,
    name,
    overview,
    runtime,
    keywords,
    cast,
    credits,
    crew,
    character,
    profile_path,
    seasons,
    season_number,
    episode_count,
    results,
    number_of_seasons,
    group_count,
    groups,
    episodes,
    episode_number,
    order,
    Last
};

enum class KodiTags {
    title = 0,
    unique_id,
    original_title,
    genre,
    year,
    original_filename,
    fanart,
    thumb,
    poster,
    aspect,
    set,
    name,
    overview,
    plot,
    outline,
    runtime,
    tag,
    actor,
    role,
    profile,
    tmdbid,
    premiered,
    season,
    episode,
    Last
};

DLL const char* KeyWords(Keys key);
DLL const char* TmdbWords(TmdbTags key, int media_type = 0);
DLL const char* KodiWords(KodiTags key);
DLL const char* XmlTagFor(MediaType media_type);
