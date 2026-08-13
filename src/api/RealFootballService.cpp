#include "api/RealFootballService.h"
#include "api/FootballApiClient.h"

namespace FootballApi {

RealFootballService::RealFootballService() {}

void RealFootballService::loadCompetitions() {
    errorMessage.clear();
    auto result = fetchCompetitions();
    if (result.success) {
        competitions = result.data;
        competitionsLoaded = true;
    } else {
        errorMessage = result.errorMessage;
    }
}

void RealFootballService::selectCompetition(int id, const std::string& code) {
    if (selectedCompetitionId != id) {
        selectedCompetitionId = id;
        selectedCompetitionCode = code;
        // Don't clear vectors immediately so UI doesn't flicker,
        // but areMatchesLoaded() will return false because ID changed.
    }
}

void RealFootballService::loadMatches() {
    if (selectedCompetitionCode.empty()) return;
    
    errorMessage.clear();
    auto result = fetchCompetitionMatches(selectedCompetitionCode);
    if (result.success) {
        matches = result.data;
        matchesLoadedForId = selectedCompetitionId;
    } else {
        errorMessage = result.errorMessage;
    }
}

void RealFootballService::loadStandings() {
    if (selectedCompetitionCode.empty()) return;
    
    errorMessage.clear();
    auto result = fetchCompetitionStandings(selectedCompetitionCode);
    if (result.success) {
        standings = result.data;
        standingsLoadedForId = selectedCompetitionId;
    } else {
        errorMessage = result.errorMessage;
    }
}

} // namespace FootballApi
