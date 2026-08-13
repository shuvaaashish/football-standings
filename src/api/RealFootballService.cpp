#include "api/RealFootballService.h"
#include "api/FootballApiClient.h"

namespace FootballApi {

RealFootballService::RealFootballService(ApiWorker& worker) : apiWorker(worker) {
}

void RealFootballService::loadCompetitions() {
    if (competitionsLoading) return;

    competitionsLoading = true;
    errorMessage.clear();

    apiWorker.postTask([this]() {
        auto result = fetchCompetitions();
        return [this, result]() {
            competitionsLoading = false;
            if (result.success) {
                competitions = result.data;
                competitionsLoaded = true;
            } else {
                errorMessage = result.errorMessage;
            }
        };
    });
}

void RealFootballService::selectCompetition(int id, const std::string& code) {
    if (selectedCompetitionId != id) {
        selectedCompetitionId = id;
        selectedCompetitionCode = code;
        // Don't clear vectors immediately so the UI does not flicker.
        // The loaded-for ID checks keep old data from being displayed.
    }
}

void RealFootballService::loadMatches() {
    if (selectedCompetitionCode.empty() || matchesLoading) return;

    const int competitionId = selectedCompetitionId;
    const std::string competitionCode = selectedCompetitionCode;
    matchesLoading = true;
    errorMessage.clear();

    apiWorker.postTask([this, competitionId, competitionCode]() {
        auto result = fetchCompetitionMatches(competitionCode);
        return [this, competitionId, result]() {
            matchesLoading = false;
            // Ignore a response for a competition the user has since changed.
            if (competitionId != selectedCompetitionId) return;

            if (result.success) {
                matches = result.data;
                matchesLoadedForId = competitionId;
            } else {
                errorMessage = result.errorMessage;
            }
        };
    });
}

void RealFootballService::loadStandings() {
    if (selectedCompetitionCode.empty() || standingsLoading) return;

    const int competitionId = selectedCompetitionId;
    const std::string competitionCode = selectedCompetitionCode;
    standingsLoading = true;
    errorMessage.clear();

    apiWorker.postTask([this, competitionId, competitionCode]() {
        auto result = fetchCompetitionStandings(competitionCode);
        return [this, competitionId, result]() {
            standingsLoading = false;
            // Ignore a response for a competition the user has since changed.
            if (competitionId != selectedCompetitionId) return;

            if (result.success) {
                standings = result.data;
                standingsLoadedForId = competitionId;
            } else {
                errorMessage = result.errorMessage;
            }
        };
    });
}

} // namespace FootballApi
