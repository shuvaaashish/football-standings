#include "Database.h"

DatabaseException::DatabaseException(const std::string& message)
    : std::runtime_error(message) {
}

LoginFailedException::LoginFailedException(const std::string& message)
    : DatabaseException(message) {
}

NotFoundException::NotFoundException(const std::string& message)
    : DatabaseException(message) {
}

ValidationException::ValidationException(const std::string& message)
    : DatabaseException(message) {
}

Database::Database(const std::string& path) : db(nullptr), databasePath(path) {
    if (sqlite3_open(databasePath.c_str(), &db) != SQLITE_OK) {
        std::string message = "Could not open database: ";
        if (db != nullptr) {
            message += sqlite3_errmsg(db);
            sqlite3_close(db);
            db = nullptr;
        }
        throw DatabaseException(message);
    }

    init();
}

Database::~Database() {
    if (db != nullptr) {
        sqlite3_close(db);
        db = nullptr;
    }
}

void Database::executeSql(const std::string& sql) const {
    char* errorMessage = nullptr;
    int result = sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errorMessage);
    if (result != SQLITE_OK) {
        std::string message = errorMessage != nullptr ? errorMessage : "Unknown SQLite error";
        sqlite3_free(errorMessage);
        throw DatabaseException(message);
    }
}

sqlite3_stmt* Database::prepareStatement(const std::string& sql) const {
    sqlite3_stmt* stmt = nullptr;
    int result = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);
    if (result != SQLITE_OK) {
        throw DatabaseException(sqlite3_errmsg(db));
    }
    return stmt;
}

int Database::getSingleInt(const std::string& sql) const {
    sqlite3_stmt* stmt = prepareStatement(sql);
    int value = 0;

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        value = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return value;
}

int Database::getCountForId(const std::string& sql, int id) const {
    sqlite3_stmt* stmt = prepareStatement(sql);
    sqlite3_bind_int(stmt, 1, id);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

int Database::getCountForTwoIds(const std::string& sql, int id1, int id2) const {
    sqlite3_stmt* stmt = prepareStatement(sql);
    sqlite3_bind_int(stmt, 1, id1);
    sqlite3_bind_int(stmt, 2, id2);

    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }

    sqlite3_finalize(stmt);
    return count;
}

std::string Database::hashPassword(const std::string& password) const {
    // Simple deterministic hash for a course project.
    // It is not meant to be strong security, only to avoid storing plain text.
    unsigned long long hash = 5381;
    for (size_t i = 0; i < password.size(); ++i) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(password[i]);
    }
    return std::to_string(hash);
}

void Database::seedDefaultUsersIfNeeded() {
    int userCount = getSingleInt("SELECT COUNT(*) FROM users;");
    if (userCount > 0) {
        return;
    }

    // Small starter accounts so the app can be tested right away.
    executeSql("INSERT INTO users (username, password_hash, role) VALUES ('admin', '" + hashPassword("admin123") + "', 'Admin');");
    executeSql("INSERT INTO users (username, password_hash, role) VALUES ('viewer', '" + hashPassword("viewer123") + "', 'Viewer');");
}

void Database::updateTeamStats(int teamId, int goalsForAdd, int goalsAgainstAdd, int winAdd, int drawAdd, int lossAdd, int pointsAdd) {
    sqlite3_stmt* stmt = prepareStatement(
        "UPDATE teams "
        "SET played = played + 1, "
        "wins = wins + ?, "
        "draws = draws + ?, "
        "losses = losses + ?, "
        "goals_for = goals_for + ?, "
        "goals_against = goals_against + ?, "
        "points = points + ? "
        "WHERE id = ?;");

    sqlite3_bind_int(stmt, 1, winAdd);
    sqlite3_bind_int(stmt, 2, drawAdd);
    sqlite3_bind_int(stmt, 3, lossAdd);
    sqlite3_bind_int(stmt, 4, goalsForAdd);
    sqlite3_bind_int(stmt, 5, goalsAgainstAdd);
    sqlite3_bind_int(stmt, 6, pointsAdd);
    sqlite3_bind_int(stmt, 7, teamId);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        throw DatabaseException("Failed to update team statistics");
    }
}

void Database::init() {
    executeSql("PRAGMA foreign_keys = ON;");

    executeSql(
        "CREATE TABLE IF NOT EXISTS leagues ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "name TEXT NOT NULL UNIQUE"
        ");");

    executeSql(
        "CREATE TABLE IF NOT EXISTS teams ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "league_id INTEGER NOT NULL, "
        "name TEXT NOT NULL, "
        "played INTEGER NOT NULL DEFAULT 0, "
        "wins INTEGER NOT NULL DEFAULT 0, "
        "draws INTEGER NOT NULL DEFAULT 0, "
        "losses INTEGER NOT NULL DEFAULT 0, "
        "goals_for INTEGER NOT NULL DEFAULT 0, "
        "goals_against INTEGER NOT NULL DEFAULT 0, "
        "points INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (league_id) REFERENCES leagues(id) ON DELETE CASCADE"
        ");");

    executeSql(
        "CREATE TABLE IF NOT EXISTS players ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "team_id INTEGER NOT NULL, "
        "name TEXT NOT NULL, "
        "position TEXT NOT NULL, "
        "goals INTEGER NOT NULL DEFAULT 0, "
        "FOREIGN KEY (team_id) REFERENCES teams(id) ON DELETE CASCADE"
        ");");

    executeSql(
        "CREATE TABLE IF NOT EXISTS matches ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "league_id INTEGER NOT NULL, "
        "home_team_id INTEGER NOT NULL, "
        "away_team_id INTEGER NOT NULL, "
        "home_score INTEGER NOT NULL, "
        "away_score INTEGER NOT NULL, "
        "match_date TEXT NOT NULL, "
        "FOREIGN KEY (league_id) REFERENCES leagues(id) ON DELETE CASCADE, "
        "FOREIGN KEY (home_team_id) REFERENCES teams(id) ON DELETE CASCADE, "
        "FOREIGN KEY (away_team_id) REFERENCES teams(id) ON DELETE CASCADE"
        ");");

    executeSql(
        "CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "username TEXT NOT NULL UNIQUE, "
        "password_hash TEXT NOT NULL, "
        "role TEXT NOT NULL CHECK(role IN ('Admin', 'Viewer'))"
        ");");

    seedDefaultUsersIfNeeded();
}

int Database::createLeague(const std::string& name) {
    if (name.empty()) {
        throw ValidationException("League name cannot be empty");
    }

    sqlite3_stmt* stmt = prepareStatement("INSERT INTO leagues (name) VALUES (?);");
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        throw DatabaseException("Could not create league");
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

int Database::addTeam(int leagueId, const std::string& name) {
    if (name.empty()) {
        throw ValidationException("Team name cannot be empty");
    }

    if (getCountForId("SELECT COUNT(*) FROM leagues WHERE id = ?;", leagueId) == 0) {
        throw NotFoundException("League not found");
    }

    sqlite3_stmt* stmt = prepareStatement("INSERT INTO teams (league_id, name) VALUES (?, ?);");
    sqlite3_bind_int(stmt, 1, leagueId);
    sqlite3_bind_text(stmt, 2, name.c_str(), -1, SQLITE_TRANSIENT);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        throw DatabaseException("Could not add team");
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

void Database::updateTeam(int teamId, const std::string& name) {
    if (name.empty()) {
        throw ValidationException("Team name cannot be empty");
    }

    if (getCountForId("SELECT COUNT(*) FROM teams WHERE id = ?;", teamId) == 0) {
        throw NotFoundException("Team not found");
    }

    sqlite3_stmt* stmt = prepareStatement("UPDATE teams SET name = ? WHERE id = ?;");
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, teamId);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        throw DatabaseException("Could not update team");
    }
}

void Database::deleteLeague(int leagueId) {
    if (getCountForId("SELECT COUNT(*) FROM leagues WHERE id = ?;", leagueId) == 0) {
        throw NotFoundException("League not found");
    }

    sqlite3_stmt* stmt = prepareStatement("DELETE FROM leagues WHERE id = ?;");
    sqlite3_bind_int(stmt, 1, leagueId);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        throw DatabaseException("Could not delete league");
    }
}

void Database::deleteTeam(int teamId) {
    if (getCountForId("SELECT COUNT(*) FROM teams WHERE id = ?;", teamId) == 0) {
        throw NotFoundException("Team not found");
    }

    sqlite3_stmt* stmt = prepareStatement("DELETE FROM teams WHERE id = ?;");
    sqlite3_bind_int(stmt, 1, teamId);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (result != SQLITE_DONE) {
        throw DatabaseException("Could not delete team");
    }
}

int Database::recordMatchResult(int leagueId, int homeTeamId, int awayTeamId, int homeScore, int awayScore, const std::string& matchDate) {
    if (homeTeamId == awayTeamId) {
        throw ValidationException("Home and away teams must be different");
    }

    if (homeScore < 0 || awayScore < 0) {
        throw ValidationException("Scores cannot be negative");
    }

    if (getCountForId("SELECT COUNT(*) FROM leagues WHERE id = ?;", leagueId) == 0) {
        throw NotFoundException("League not found");
    }

    if (getCountForTwoIds("SELECT COUNT(*) FROM teams WHERE id = ? AND league_id = ?;", homeTeamId, leagueId) == 0) {
        throw NotFoundException("Home team not found in this league");
    }

    if (getCountForTwoIds("SELECT COUNT(*) FROM teams WHERE id = ? AND league_id = ?;", awayTeamId, leagueId) == 0) {
        throw NotFoundException("Away team not found in this league");
    }

    executeSql("BEGIN TRANSACTION;");

    try {
        sqlite3_stmt* stmt = prepareStatement(
            "INSERT INTO matches (league_id, home_team_id, away_team_id, home_score, away_score, match_date) "
            "VALUES (?, ?, ?, ?, ?, ?);");
        sqlite3_bind_int(stmt, 1, leagueId);
        sqlite3_bind_int(stmt, 2, homeTeamId);
        sqlite3_bind_int(stmt, 3, awayTeamId);
        sqlite3_bind_int(stmt, 4, homeScore);
        sqlite3_bind_int(stmt, 5, awayScore);
        sqlite3_bind_text(stmt, 6, matchDate.c_str(), -1, SQLITE_TRANSIENT);

        int result = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        if (result != SQLITE_DONE) {
            throw DatabaseException("Could not record match result");
        }

        if (homeScore > awayScore) {
            updateTeamStats(homeTeamId, homeScore, awayScore, 1, 0, 0, 3);
            updateTeamStats(awayTeamId, awayScore, homeScore, 0, 0, 1, 0);
        } else if (homeScore < awayScore) {
            updateTeamStats(homeTeamId, homeScore, awayScore, 0, 0, 1, 0);
            updateTeamStats(awayTeamId, awayScore, homeScore, 1, 0, 0, 3);
        } else {
            updateTeamStats(homeTeamId, homeScore, awayScore, 0, 1, 0, 1);
            updateTeamStats(awayTeamId, awayScore, homeScore, 0, 1, 0, 1);
        }

        executeSql("COMMIT;");
    } catch (...) {
        executeSql("ROLLBACK;");
        throw;
    }

    return static_cast<int>(sqlite3_last_insert_rowid(db));
}

std::vector<League> Database::getLeagues() const {
    std::vector<League> leagues;
    sqlite3_stmt* stmt = prepareStatement("SELECT id, name FROM leagues ORDER BY name ASC;");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int leagueId = sqlite3_column_int(stmt, 0);
        std::string leagueName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        League league(leagueName, leagueId);
        leagues.push_back(league);
    }

    sqlite3_finalize(stmt);
    return leagues;
}

std::vector<Team> Database::getTeams(int leagueId) const {
    std::vector<Team> teams;
    sqlite3_stmt* stmt = prepareStatement(
        "SELECT id, league_id, name, played, wins, draws, losses, goals_for, goals_against, points "
        "FROM teams WHERE league_id = ? ORDER BY name ASC;");
    sqlite3_bind_int(stmt, 1, leagueId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int teamId = sqlite3_column_int(stmt, 0);
        std::string teamName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        int played = sqlite3_column_int(stmt, 3);
        int wins = sqlite3_column_int(stmt, 4);
        int draws = sqlite3_column_int(stmt, 5);
        int losses = sqlite3_column_int(stmt, 6);
        int goalsFor = sqlite3_column_int(stmt, 7);
        int goalsAgainst = sqlite3_column_int(stmt, 8);
        int points = sqlite3_column_int(stmt, 9);
        Team team(teamName, teamId, played, wins, draws, losses, goalsFor, goalsAgainst, points);
        teams.push_back(team);
    }

    sqlite3_finalize(stmt);
    return teams;
}

std::vector<Team> Database::getStandings(int leagueId) const {
    std::vector<Team> standings;
    sqlite3_stmt* stmt = prepareStatement(
        "SELECT id, name, played, wins, draws, losses, goals_for, goals_against, points "
        "FROM teams WHERE league_id = ? "
        "ORDER BY points DESC, (goals_for - goals_against) DESC, goals_for DESC, name ASC;");
    sqlite3_bind_int(stmt, 1, leagueId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int teamId = sqlite3_column_int(stmt, 0);
        std::string teamName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        int played = sqlite3_column_int(stmt, 2);
        int wins = sqlite3_column_int(stmt, 3);
        int draws = sqlite3_column_int(stmt, 4);
        int losses = sqlite3_column_int(stmt, 5);
        int goalsFor = sqlite3_column_int(stmt, 6);
        int goalsAgainst = sqlite3_column_int(stmt, 7);
        int points = sqlite3_column_int(stmt, 8);
        Team team(teamName, teamId, played, wins, draws, losses, goalsFor, goalsAgainst, points);
        standings.push_back(team);
    }

    sqlite3_finalize(stmt);
    return standings;
}

std::vector<Match> Database::getMatches(int leagueId) const {
    std::vector<Match> matches;
    sqlite3_stmt* stmt = prepareStatement(
        "SELECT m.id, m.league_id, m.home_team_id, m.away_team_id, "
        "ht.name, at.name, m.home_score, m.away_score, m.match_date "
        "FROM matches m "
        "JOIN teams ht ON m.home_team_id = ht.id "
        "JOIN teams at ON m.away_team_id = at.id "
        "WHERE m.league_id = ? "
        "ORDER BY m.match_date DESC, m.id DESC;");
    sqlite3_bind_int(stmt, 1, leagueId);

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string homeTeamName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
        std::string awayTeamName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
        int homeScore = sqlite3_column_int(stmt, 6);
        int awayScore = sqlite3_column_int(stmt, 7);
        std::string matchDate = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8));
        Match match(homeTeamName, awayTeamName, homeScore, awayScore, matchDate);
        matches.push_back(match);
    }

    sqlite3_finalize(stmt);
    return matches;
}

bool Database::tableExists(const std::string& tableName) const {
    sqlite3_stmt* stmt = prepareStatement(
        "SELECT COUNT(*) FROM sqlite_master WHERE type = 'table' AND name = ?;");
    sqlite3_bind_text(stmt, 1, tableName.c_str(), -1, SQLITE_TRANSIENT);

    bool exists = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        exists = (sqlite3_column_int(stmt, 0) > 0);
    }

    sqlite3_finalize(stmt);
    return exists;
}

User* Database::authenticateUser(const std::string& username, const std::string& password) const {
    sqlite3_stmt* stmt = prepareStatement("SELECT role, password_hash FROM users WHERE username = ?;");
    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_TRANSIENT);

    User* user = nullptr;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string storedHash = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string inputHash = hashPassword(password);

        if (storedHash != inputHash) {
            sqlite3_finalize(stmt);
            throw LoginFailedException("Login failed");
        }

        if (role == "Admin") {
            user = new Admin(username);
        } else if (role == "Viewer") {
            user = new Viewer(username);
        } else {
            sqlite3_finalize(stmt);
            throw DatabaseException("Unknown user role in database");
        }
    } else {
        sqlite3_finalize(stmt);
        throw LoginFailedException("Login failed");
    }

    sqlite3_finalize(stmt);
    return user;
}
