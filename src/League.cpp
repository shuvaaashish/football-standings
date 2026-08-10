#include "League.h"

League::League(const std::string& name) : id(-1), name(name) {
}

League::League(const std::string& name, int id) : id(id), name(name) {
}

int League::getId() const {
    return id;
}

void League::setId(int newId) {
    id = newId;
}

std::string League::getName() const {
    return name;
}

void League::setName(const std::string& newName) {
    name = newName;
}

void League::addTeam(const Team& team) {
    teams.push_back(team);
}

void League::removeTeam(const std::string& teamName) {
    for (std::vector<Team>::iterator it = teams.begin(); it != teams.end(); ++it) {
        if (it->getName() == teamName) {
            teams.erase(it);
            return;
        }
    }
}

std::vector<Team> League::getTeams() const {
    return teams;
}

std::vector<Team> League::getSortedStandings() const {
    std::vector<Team> sortedTeams = teams;

    for (size_t i = 0; i < sortedTeams.size(); ++i) {
        for (size_t j = 0; j + 1 < sortedTeams.size() - i; ++j) {
            if (sortedTeams[j + 1] < sortedTeams[j]) {
                Team temp = sortedTeams[j];
                sortedTeams[j] = sortedTeams[j + 1];
                sortedTeams[j + 1] = temp;
            }
        }
    }

    return sortedTeams;
}