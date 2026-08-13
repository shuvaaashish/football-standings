#pragma once

#include <string>
#include <vector>
#include <optional>

// Separate namespace for real-world football data from football-data.org.
// These are completely independent of the existing League/Team/Match classes.

namespace RealFootball {

struct RealTeam {
    int id = 0;
    std::string name;
    std::string shortName;
    std::string tla;        // Three-Letter Abbreviation (e.g. "ARS")
    std::string crestUrl;
};

struct RealCompetition {
    int id = 0;
    std::string name;
    std::string code;       // e.g. "PL", "CL", "BL1"
    std::string type;       // e.g. "LEAGUE", "CUP"
    std::string emblemUrl;
    std::string areaName;   // e.g. "England", "Europe"
};

struct RealMatch {
    int id = 0;
    int competitionId = 0;
    std::string utcDate;    // ISO 8601 date string
    std::string status;     // e.g. "SCHEDULED", "FINISHED", "IN_PLAY", "TIMED"
    int matchday = 0;

    RealTeam homeTeam;
    RealTeam awayTeam;

    // Scores are optional because scheduled/future matches have null scores
    std::optional<int> homeScoreFullTime;
    std::optional<int> awayScoreFullTime;
};

struct RealStanding {
    int position = 0;
    RealTeam team;
    int playedGames = 0;
    int won = 0;
    int draw = 0;
    int lost = 0;
    int goalsFor = 0;
    int goalsAgainst = 0;
    int goalDifference = 0;
    int points = 0;
};

// Generic result wrapper for API calls.
// Prevents exceptions from crashing the app on network/JSON errors.
template<typename T>
struct ApiResult {
    bool success = false;
    std::string errorMessage;
    T data;
};

} // namespace RealFootball
