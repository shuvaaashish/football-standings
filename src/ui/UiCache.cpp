#include "UiCache.h"

UiCache::UiCache() {
}

void UiCache::refreshLeagues(Database& db) {
    std::lock_guard<std::mutex> lg(mutex);
    leagues = db.getLeagues();
}

void UiCache::refreshTeams(Database& db, int leagueId) {
    std::lock_guard<std::mutex> lg(mutex);
    teamsByLeague[leagueId] = db.getTeams(leagueId);
}

void UiCache::refreshMatches(Database& db, int leagueId) {
    std::lock_guard<std::mutex> lg(mutex);
    matchesByLeague[leagueId] = db.getMatches(leagueId);
}

const std::vector<League>& UiCache::getLeagues() const {
    std::lock_guard<std::mutex> lg(mutex);
    return leagues;
}

const std::vector<Team>& UiCache::getTeams(int leagueId) const {
    std::lock_guard<std::mutex> lg(mutex);
    static const std::vector<Team> empty;
    auto it = teamsByLeague.find(leagueId);
    if (it == teamsByLeague.end()) return empty;
    return it->second;
}

const std::vector<Match>& UiCache::getMatches(int leagueId) const {
    std::lock_guard<std::mutex> lg(mutex);
    static const std::vector<Match> empty;
    auto it = matchesByLeague.find(leagueId);
    if (it == matchesByLeague.end()) return empty;
    return it->second;
}

void UiCache::invalidate() {
    std::lock_guard<std::mutex> lg(mutex);
    leagues.clear();
    teamsByLeague.clear();
    matchesByLeague.clear();
}
