#ifndef TEAM_H
#define TEAM_H

#include <iostream>
#include <string>

// Simple data-and-behavior class for one football team.

class Team {
private:
    std::string name;
    int played;
    int wins;
    int draws;
    int losses;
    int goalsFor;
    int goalsAgainst;
    int points;

public:
    Team(const std::string& name,
         int played = 0,
         int wins = 0,
         int draws = 0,
         int losses = 0,
         int goalsFor = 0,
         int goalsAgainst = 0,
         int points = 0);

    std::string getName() const;
    int getPlayed() const;
    int getWins() const;
    int getDraws() const;
    int getLosses() const;
    int getGoalsFor() const;
    int getGoalsAgainst() const;
    int getGoalDifference() const;
    int getPoints() const;

    // Updates the team's stats after one match.
    void recordResult(int goalsScored, int goalsConceded);

    // For standings sorting: higher points first, then higher goal difference.
    bool operator<(const Team& other) const;

    // Simple display output for tables/debugging.
    friend std::ostream& operator<<(std::ostream& out, const Team& team);
};

#endif