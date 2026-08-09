#ifndef MATCH_H
#define MATCH_H

#include <string>


class Match {
private:
    std::string homeTeamName;
    std::string awayTeamName;
    int homeScore;
    int awayScore;
    std::string matchDate;

public:
    Match(const std::string& homeTeamName,
          const std::string& awayTeamName,
          int homeScore,
          int awayScore,
          const std::string& matchDate);

    std::string getHomeTeamName() const;
    std::string getAwayTeamName() const;
    int getHomeScore() const;
    int getAwayScore() const;
    std::string getMatchDate() const;
};

#endif