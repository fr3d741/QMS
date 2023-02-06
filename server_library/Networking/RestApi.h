#pragma once

#include "Macros.h"
#include <CommonDefines.h>
#include <Logging/ILogger.h>

class DLL RestApi
{
public:
    static QString& ApiKey();
    static QString SearchMovieWithoutYear(const QString& title, Logging::ILogger::Ptr log);
    static QString SearchMovie(const QString& title, Logging::ILogger::Ptr log);
    static QString SearchTvWithoutYear(const QString& title, Logging::ILogger::Ptr log);
    static QString SearchTv(const QString& title, Logging::ILogger::Ptr log);

    static QString MovieDetails(const QString& id, Logging::ILogger::Ptr log);
    static QString TvDetails(const QString& id, Logging::ILogger::Ptr log);
    static QString EpisodeGroups(const QString& id, Logging::ILogger::Ptr log);
    static QString EpisodeGroup(const QString& group_id, Logging::ILogger::Ptr log);
    static QString Season(const QString& id, int season_nr, Logging::ILogger::Ptr log);
};
