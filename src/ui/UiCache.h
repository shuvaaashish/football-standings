#ifndef UICACHE_H
#define UICACHE_H

#include <vector>
#include <mutex>
#include <unordered_map>
#include "League.h"
#include "Team.h"
#include "Match.h"
#include "Database.h"

class UiCache {
public:
    UiCache();

    void refreshLeagues(Database& db);
    void refreshTeams(Database& db, int leagueId);
    void refreshMatches(Database& db, int leagueId);

    const std::vector<League>& getLeagues() const;
    const std::vector<Team>& getTeams(int leagueId) const;
    const std::vector<Match>& getMatches(int leagueId) const;

    void invalidate();

private:
    std::vector<League> leagues;
    std::unordered_map<int, std::vector<Team>> teamsByLeague;
    std::unordered_map<int, std::vector<Match>> matchesByLeague;

    mutable std::mutex mutex;
};

#endif
