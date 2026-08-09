#include "Match.h"

Match::Match(const std::string& homeTeamName,
             const std::string& awayTeamName,
             int homeScore,
             int awayScore,
             const std::string& matchDate)
    : homeTeamName(homeTeamName), awayTeamName(awayTeamName), homeScore(homeScore), awayScore(awayScore), matchDate(matchDate) {
}

std::string Match::getHomeTeamName() const {
    return homeTeamName;
}

std::string Match::getAwayTeamName() const {
    return awayTeamName;
}

int Match::getHomeScore() const {
    return homeScore;
}

int Match::getAwayScore() const {
    return awayScore;
}

std::string Match::getMatchDate() const {
    return matchDate;
}