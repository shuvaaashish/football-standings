#include "Team.h"

Team::Team(const std::string& name,
           int played,
           int wins,
           int draws,
           int losses,
           int goalsFor,
           int goalsAgainst,
           int points)
    : name(name), played(played), wins(wins), draws(draws), losses(losses), goalsFor(goalsFor), goalsAgainst(goalsAgainst), points(points) {
}

std::string Team::getName() const {
    return name;
}

int Team::getPlayed() const {
    return played;
}

int Team::getWins() const {
    return wins;
}

int Team::getDraws() const {
    return draws;
}

int Team::getLosses() const {
    return losses;
}

int Team::getGoalsFor() const {
    return goalsFor;
}

int Team::getGoalsAgainst() const {
    return goalsAgainst;
}

int Team::getGoalDifference() const {
    return goalsFor - goalsAgainst;
}

int Team::getPoints() const {
    return points;
}

void Team::recordResult(int goalsScored, int goalsConceded) {
    if (goalsScored < 0 || goalsConceded < 0) {
        return;
    }

    ++played;
    goalsFor += goalsScored;
    goalsAgainst += goalsConceded;

    if (goalsScored > goalsConceded) {
        ++wins;
        points += 3;
    } else if (goalsScored == goalsConceded) {
        ++draws;
        ++points;
    } else {
        ++losses;
    }
}

bool Team::operator<(const Team& other) const {
    if (points != other.points) {
        return points > other.points;
    }

    int thisGD = getGoalDifference();
    int otherGD = other.getGoalDifference();
    if (thisGD != otherGD) {
        return thisGD > otherGD;
    }

    return name < other.name;
}

std::ostream& operator<<(std::ostream& out, const Team& team) {
    out << team.name << " | P:" << team.played
        << " W:" << team.wins
        << " D:" << team.draws
        << " L:" << team.losses
        << " GF:" << team.goalsFor
        << " GA:" << team.goalsAgainst
        << " GD:" << team.getGoalDifference()
        << " Pts:" << team.points;
    return out;
}