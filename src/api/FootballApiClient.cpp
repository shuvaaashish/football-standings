#include "api/FootballApiClient.h"
#include "../../config/ApiConfig.local.h"
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
    // Callback function for libcurl to write received data into a std::string
    size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        size_t totalSize = size * nmemb;
        std::string* s = static_cast<std::string*>(userp);
        s->append(static_cast<char*>(contents), totalSize);
        return totalSize;
    }

    // Returns the API key string, or empty if not configured.
    std::string getApiKey() {
        std::string key = FOOTBALL_API_KEY;
        if (key == "YOUR_API_KEY_HERE" || key.empty()) return "";
        return key;
    }

    // Internal reusable GET request helper.
    // Makes an authenticated GET to https://api.football-data.org/v4/<endpoint>.
    // On success, writes the response body into 'outBody' and returns true.
    // On failure, writes a human-readable message into 'outError' and returns false.
    bool makeGetRequest(const std::string& endpoint, std::string& outBody, std::string& outError) {
        std::string apiKey = getApiKey();
        if (apiKey.empty()) {
            outError = "API token is missing or not set. Please edit config/ApiConfig.local.h.";
            return false;
        }

        CURL* curl = curl_easy_init();
        if (!curl) {
            outError = "Failed to initialize cURL.";
            return false;
        }

        std::string url = "https://api.football-data.org/v4/" + endpoint;
        std::string readBuffer;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        struct curl_slist* headers = NULL;
        std::string authHeader = "X-Auth-Token: " + apiKey;
        headers = curl_slist_append(headers, authHeader.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

        // API endpoints often drop connections without a User-Agent, leading to timeouts.
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "FootballHub/1.0");
        // Set a reasonable timeout so it doesn't hang indefinitely.
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);

        CURLcode res = curl_easy_perform(curl);

        bool success = false;
        if (res != CURLE_OK) {
            outError = std::string("cURL failed: ") + curl_easy_strerror(res);
        } else {
            long response_code;
            curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

            if (response_code == 200) {
                outBody = readBuffer;
                success = true;
            } else if (response_code == 403 || response_code == 400) {
                outError = "HTTP " + std::to_string(response_code) + " - Authentication failed or bad request.";
            } else if (response_code == 429) {
                outError = "HTTP 429 - Rate limit exceeded. Try again later.";
            } else {
                outError = "HTTP Error " + std::to_string(response_code);
            }
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return success;
    }
} // anonymous namespace


namespace FootballApi {

// ============================================================================
// fetchCompetitions
// Endpoint: GET /v4/competitions
// ============================================================================
RealFootball::ApiResult<std::vector<RealFootball::RealCompetition>>
fetchCompetitions() {
    RealFootball::ApiResult<std::vector<RealFootball::RealCompetition>> result;

    std::string body, error;
    if (!makeGetRequest("competitions", body, error)) {
        result.errorMessage = error;
        return result;
    }

    try {
        json j = json::parse(body);
        if (j.contains("competitions") && j["competitions"].is_array()) {
            for (const auto& comp : j["competitions"]) {
                RealFootball::RealCompetition c;
                c.id   = comp.value("id", 0);
                c.name = comp.value("name", "Unknown");
                c.code = comp.value("code", "");
                c.type = comp.value("type", "");
                c.emblemUrl = comp.value("emblem", "");
                if (comp.contains("area") && comp["area"].is_object()) {
                    c.areaName = comp["area"].value("name", "");
                }
                result.data.push_back(c);
            }
        }
        result.success = true;
    } catch (const json::exception& e) {
        result.errorMessage = std::string("JSON parse error: ") + e.what();
    }
    return result;
}

// ============================================================================
// fetchCompetitionMatches
// Endpoint: GET /v4/competitions/{code}/matches
// ============================================================================
RealFootball::ApiResult<std::vector<RealFootball::RealMatch>>
fetchCompetitionMatches(const std::string& competitionCode) {
    RealFootball::ApiResult<std::vector<RealFootball::RealMatch>> result;

    std::string body, error;
    if (!makeGetRequest("competitions/" + competitionCode + "/matches", body, error)) {
        result.errorMessage = error;
        return result;
    }

    try {
        json j = json::parse(body);
        if (j.contains("matches") && j["matches"].is_array()) {
            for (const auto& m : j["matches"]) {
                RealFootball::RealMatch match;
                match.id       = (m.contains("id") && m["id"].is_number()) ? m["id"].get<int>() : 0;
                match.utcDate  = (m.contains("utcDate") && m["utcDate"].is_string()) ? m["utcDate"].get<std::string>() : "";
                match.status   = (m.contains("status") && m["status"].is_string()) ? m["status"].get<std::string>() : "";
                match.matchday = (m.contains("matchday") && m["matchday"].is_number()) ? m["matchday"].get<int>() : 0;

                if (m.contains("competition") && m["competition"].is_object()) {
                    match.competitionId = m["competition"].value("id", 0);
                }

                // Parse home team
                if (m.contains("homeTeam") && m["homeTeam"].is_object()) {
                    const auto& ht = m["homeTeam"];
                    match.homeTeam.id        = (ht.contains("id") && ht["id"].is_number()) ? ht["id"].get<int>() : 0;
                    match.homeTeam.name      = (ht.contains("name") && ht["name"].is_string()) ? ht["name"].get<std::string>() : "Unknown";
                    match.homeTeam.shortName = (ht.contains("shortName") && ht["shortName"].is_string()) ? ht["shortName"].get<std::string>() : "";
                    match.homeTeam.tla       = (ht.contains("tla") && ht["tla"].is_string()) ? ht["tla"].get<std::string>() : "";
                    match.homeTeam.crestUrl  = (ht.contains("crest") && ht["crest"].is_string()) ? ht["crest"].get<std::string>() : "";
                }

                // Parse away team
                if (m.contains("awayTeam") && m["awayTeam"].is_object()) {
                    const auto& at = m["awayTeam"];
                    match.awayTeam.id        = (at.contains("id") && at["id"].is_number()) ? at["id"].get<int>() : 0;
                    match.awayTeam.name      = (at.contains("name") && at["name"].is_string()) ? at["name"].get<std::string>() : "Unknown";
                    match.awayTeam.shortName = (at.contains("shortName") && at["shortName"].is_string()) ? at["shortName"].get<std::string>() : "";
                    match.awayTeam.tla       = (at.contains("tla") && at["tla"].is_string()) ? at["tla"].get<std::string>() : "";
                    match.awayTeam.crestUrl  = (at.contains("crest") && at["crest"].is_string()) ? at["crest"].get<std::string>() : "";
                }

                // Parse scores — safely handle null values with std::optional
                if (m.contains("score") && m["score"].is_object()) {
                    const auto& score = m["score"];
                    if (score.contains("fullTime") && score["fullTime"].is_object()) {
                        const auto& ft = score["fullTime"];
                        if (ft.contains("home") && !ft["home"].is_null()) {
                            match.homeScoreFullTime = ft["home"].get<int>();
                        }
                        if (ft.contains("away") && !ft["away"].is_null()) {
                            match.awayScoreFullTime = ft["away"].get<int>();
                        }
                    }
                }

                result.data.push_back(match);
            }
        }
        result.success = true;
    } catch (const json::exception& e) {
        result.errorMessage = std::string("JSON parse error: ") + e.what();
    }
    return result;
}

// ============================================================================
// fetchCompetitionStandings
// Endpoint: GET /v4/competitions/{code}/standings
// ============================================================================
RealFootball::ApiResult<std::vector<RealFootball::RealStanding>>
fetchCompetitionStandings(const std::string& competitionCode) {
    RealFootball::ApiResult<std::vector<RealFootball::RealStanding>> result;

    std::string body, error;
    if (!makeGetRequest("competitions/" + competitionCode + "/standings", body, error)) {
        result.errorMessage = error;
        return result;
    }

    try {
        json j = json::parse(body);
        // World Cup-style: one standings[] element per group, each with "group" + "table".
        // League-style: a single element with no "group" field. Loop every element — do not filter by type.
        if (j.contains("standings") && j["standings"].is_array()) {
            for (const auto& standingBlock : j["standings"]) {
                if (!standingBlock.contains("table") || !standingBlock["table"].is_array()) continue;

                std::string groupName;
                if (standingBlock.contains("group") && standingBlock["group"].is_string()) {
                    groupName = standingBlock["group"].get<std::string>();
                }

                for (const auto& entry : standingBlock["table"]) {
                    RealFootball::RealStanding s;
                    s.groupName      = groupName;
                    s.position       = (entry.contains("position") && entry["position"].is_number()) ? entry["position"].get<int>() : 0;
                    s.playedGames    = (entry.contains("playedGames") && entry["playedGames"].is_number()) ? entry["playedGames"].get<int>() : 0;
                    s.won            = (entry.contains("won") && entry["won"].is_number()) ? entry["won"].get<int>() : 0;
                    s.draw           = (entry.contains("draw") && entry["draw"].is_number()) ? entry["draw"].get<int>() : 0;
                    s.lost           = (entry.contains("lost") && entry["lost"].is_number()) ? entry["lost"].get<int>() : 0;
                    s.goalsFor       = (entry.contains("goalsFor") && entry["goalsFor"].is_number()) ? entry["goalsFor"].get<int>() : 0;
                    s.goalsAgainst   = (entry.contains("goalsAgainst") && entry["goalsAgainst"].is_number()) ? entry["goalsAgainst"].get<int>() : 0;
                    s.goalDifference = (entry.contains("goalDifference") && entry["goalDifference"].is_number()) ? entry["goalDifference"].get<int>() : 0;
                    s.points         = (entry.contains("points") && entry["points"].is_number()) ? entry["points"].get<int>() : 0;

                    if (entry.contains("team") && entry["team"].is_object()) {
                        const auto& t = entry["team"];
                        s.team.id        = (t.contains("id") && t["id"].is_number()) ? t["id"].get<int>() : 0;
                        s.team.name      = (t.contains("name") && t["name"].is_string()) ? t["name"].get<std::string>() : "Unknown";
                        s.team.shortName = (t.contains("shortName") && t["shortName"].is_string()) ? t["shortName"].get<std::string>() : "";
                        s.team.tla       = (t.contains("tla") && t["tla"].is_string()) ? t["tla"].get<std::string>() : "";
                        s.team.crestUrl  = (t.contains("crest") && t["crest"].is_string()) ? t["crest"].get<std::string>() : "";
                    }

                    result.data.push_back(s);
                }
            }
        }
        result.success = true;
    } catch (const json::exception& e) {
        result.errorMessage = std::string("JSON parse error: ") + e.what();
    }
    return result;
}

// ============================================================================
// runApiConnectionTest — Updated for Step 4
// Demonstrates that all three fetch functions produce valid C++ objects.
// ============================================================================
void runApiConnectionTest() {
    std::cout << "\n====================================\n";
    std::cout << "  REAL FOOTBALL DATA MODEL TEST\n";
    std::cout << "====================================\n\n";

    // --- 1. Fetch Competitions ---
    std::cout << "Fetching competitions...\n\n";
    auto compResult = fetchCompetitions();

    if (!compResult.success) {
        std::cout << "Football API Error: " << compResult.errorMessage << "\n";
        std::cout << "====================================\n\n";
        return;
    }

    std::cout << "Competitions received: " << compResult.data.size() << "\n\n";

    int printed = 0;
    for (const auto& c : compResult.data) {
        if (printed >= 3) break;
        std::cout << ++printed << ". " << c.name << "\n"
                  << "   ID: " << c.id << "\n"
                  << "   Code: " << c.code << "\n"
                  << "   Country: " << c.areaName << "\n\n";
    }

    // --- 2. Fetch Matches for Premier League ---
    std::cout << "------------------------------------\n\n";
    std::cout << "Fetching matches for PL (Premier League)...\n\n";

    auto matchResult = fetchCompetitionMatches("PL");
    if (!matchResult.success) {
        std::cout << "Matches Error: " << matchResult.errorMessage << "\n\n";
    } else {
        std::cout << "Matches received: " << matchResult.data.size() << "\n\n";

        // Print up to 3 finished matches
        int finishedPrinted = 0;
        for (const auto& m : matchResult.data) {
            if (finishedPrinted >= 3) break;
            if (m.status == "FINISHED") {
                std::cout << m.homeTeam.name << " "
                          << m.homeScoreFullTime.value_or(0) << " - "
                          << m.awayScoreFullTime.value_or(0) << " "
                          << m.awayTeam.name << "\n"
                          << "  Date: " << m.utcDate
                          << "  Status: " << m.status << "\n\n";
                finishedPrinted++;
            }
        }

        // Print up to 2 scheduled matches (proving null score handling)
        int scheduledPrinted = 0;
        for (const auto& m : matchResult.data) {
            if (scheduledPrinted >= 2) break;
            if (m.status == "TIMED" || m.status == "SCHEDULED") {
                std::cout << m.homeTeam.name << " vs " << m.awayTeam.name << "\n"
                          << "  Date: " << m.utcDate
                          << "  Status: " << m.status
                          << "  Score: not available\n\n";
                scheduledPrinted++;
            }
        }
    }

    // --- 3. Fetch Standings for Premier League ---
    std::cout << "------------------------------------\n\n";
    std::cout << "Fetching standings for PL (Premier League)...\n\n";

    auto standResult = fetchCompetitionStandings("PL");
    if (!standResult.success) {
        std::cout << "Standings Error: " << standResult.errorMessage << "\n\n";
    } else {
        std::cout << "Standings entries: " << standResult.data.size() << "\n\n";
        int standPrinted = 0;
        for (const auto& s : standResult.data) {
            if (standPrinted >= 5) break;
            std::cout << s.position << ". " << s.team.name << "\n"
                      << "   Played: " << s.playedGames
                      << "  W: " << s.won
                      << "  D: " << s.draw
                      << "  L: " << s.lost
                      << "  GD: " << s.goalDifference
                      << "  Points: " << s.points << "\n\n";
            standPrinted++;
        }
    }

    std::cout << "====================================\n\n";
}

} // namespace FootballApi
