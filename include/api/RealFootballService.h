#pragma once

#include "api/RealFootballModels.h"
#include <vector>
#include <string>

namespace FootballApi {

class RealFootballService {
public:
    RealFootballService();

    void loadCompetitions();
    void selectCompetition(int id, const std::string& code);
    void loadMatches();
    void loadStandings();

    const std::vector<RealFootball::RealCompetition>& getCompetitions() const { return competitions; }
    const std::vector<RealFootball::RealMatch>& getMatches() const { return matches; }
    const std::vector<RealFootball::RealStanding>& getStandings() const { return standings; }

    bool areCompetitionsLoaded() const { return competitionsLoaded; }
    int getSelectedCompetitionId() const { return selectedCompetitionId; }
    const std::string& getSelectedCompetitionCode() const { return selectedCompetitionCode; }
    const std::string& getErrorMessage() const { return errorMessage; }
    
    // Check if matches or standings are loaded for the current selected competition
    bool areMatchesLoaded() const { return matchesLoadedForId == selectedCompetitionId && selectedCompetitionId != 0; }
    bool areStandingsLoaded() const { return standingsLoadedForId == selectedCompetitionId && selectedCompetitionId != 0; }

private:
    std::vector<RealFootball::RealCompetition> competitions;
    std::vector<RealFootball::RealMatch> matches;
    std::vector<RealFootball::RealStanding> standings;

    bool competitionsLoaded = false;
    int selectedCompetitionId = 0;
    std::string selectedCompetitionCode;
    
    int matchesLoadedForId = 0;
    int standingsLoadedForId = 0;

    std::string errorMessage;
};

} // namespace FootballApi
