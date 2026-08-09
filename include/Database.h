#ifndef DATABASE_H
#define DATABASE_H

#include "League.h"
#include "Match.h"
#include "Team.h"
#include "User.h"
#include <sqlite3.h>
#include <stdexcept>
#include <string>
#include <vector>

// Small custom exceptions so the UI can show simple error messages.
class DatabaseException : public std::runtime_error {
public:
    DatabaseException(const std::string& message);
};

class LoginFailedException : public DatabaseException {
public:
    LoginFailedException(const std::string& message);
};

class NotFoundException : public DatabaseException {
public:
    NotFoundException(const std::string& message);
};

class ValidationException : public DatabaseException {
public:
    ValidationException(const std::string& message);
};

class Database {
private:
    sqlite3* db;
    std::string databasePath;

    void init();
    void executeSql(const std::string& sql) const;
    sqlite3_stmt* prepareStatement(const std::string& sql) const;
    int getSingleInt(const std::string& sql) const;
    int getCountForId(const std::string& sql, int id) const;
    int getCountForTwoIds(const std::string& sql, int id1, int id2) const;
    std::string hashPassword(const std::string& password) const;
    void seedDefaultUsersIfNeeded();
    void updateTeamStats(int teamId, int goalsForAdd, int goalsAgainstAdd, int winAdd, int drawAdd, int lossAdd, int pointsAdd);

public:
    Database(const std::string& path = "data/football_standings.db");
    ~Database();

    int createLeague(const std::string& name);
    int addTeam(int leagueId, const std::string& name);
    void updateTeam(int teamId, const std::string& name);
    void deleteTeam(int teamId);
    int recordMatchResult(int leagueId, int homeTeamId, int awayTeamId, int homeScore, int awayScore, const std::string& matchDate);

    std::vector<League> getLeagues() const;
    std::vector<Team> getTeams(int leagueId) const;
    std::vector<Team> getStandings(int leagueId) const;
    std::vector<Match> getMatches(int leagueId) const;
    bool tableExists(const std::string& tableName) const;

    User* authenticateUser(const std::string& username, const std::string& password) const;
};

#endif
