#pragma once

#include "Macros.h"
#include <CommonDefines.h>

class DLL RestApi
{
public:
    static QString& ApiKey();
    static QString SearchMovie(const QString& title);
    static QString SearchTv(const QString& title);

    static QString MovieDetails(const QString& id);
    static QString TvDetails(const QString& id);
    static QString EpisodeGroups(const QString& id);
    static QString EpisodeGroup(const QString& group_id);
    static QString Season(const QString& id, int season_nr);
};

