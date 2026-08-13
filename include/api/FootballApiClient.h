#pragma once

#include "api/RealFootballModels.h"
#include <vector>
#include <string>

namespace FootballApi {

    // Step 3 proof-of-concept: prints API test results to console.
    void runApiConnectionTest();

    // Step 4 typed fetch functions:
    RealFootball::ApiResult<std::vector<RealFootball::RealCompetition>>
    fetchCompetitions();

    RealFootball::ApiResult<std::vector<RealFootball::RealMatch>>
    fetchCompetitionMatches(const std::string& competitionCode);

    RealFootball::ApiResult<std::vector<RealFootball::RealStanding>>
    fetchCompetitionStandings(const std::string& competitionCode);
}
