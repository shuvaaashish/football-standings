#ifndef LEAGUE_H
#define LEAGUE_H

#include "Team.h"
#include <string>
#include <vector>

class League {
private:
    std::string name;
    std::vector<Team> teams;

public:
    League(const std::string& name = "");

    std::string getName() const;
    void setName(const std::string& newName);

    void addTeam(const Team& team);
    void removeTeam(const std::string& teamName);

    std::vector<Team> getTeams() const;
    std::vector<Team> getSortedStandings() const;
};

#endif