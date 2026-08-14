#ifndef UI_STATE_H
#define UI_STATE_H

#include <string>

namespace UI {

struct ViewerUiState {
    int selectedLeagueIndex = -1;
    std::string errorMessage;
};

struct AdminUiState {
    char newLeagueName[128];
    char newTeamName[128];
    int selectedLeagueForTeams = 0;
    int selectedLeagueForMatches = 0;
    int selectedLeagueForStandings = 0;
    int selectedHomeTeam = 0;
    int selectedAwayTeam = 0;
    int homeScore = 0;
    int awayScore = 0;
    char matchDateBuffer[16] = "";
    std::string errorMessage;
    std::string successMessage;

    AdminUiState() {
        newLeagueName[0] = '\0';
        newTeamName[0] = '\0';
    }
};

} // namespace UI

#endif // UI_STATE_H
