#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Database.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>

struct LoginState {
    bool loggedIn;
    bool showSignUp;
    User* currentUser;
    char username[128];
    char password[128];
    char signupUsername[128];
    char signupPassword[128];
    char signupConfirm[128];
    std::string errorMessage;
    std::string signupErrorMessage;
    std::string signupSuccessMessage;

    LoginState() : loggedIn(false), showSignUp(false), currentUser(nullptr) {
        username[0] = '\0';
        password[0] = '\0';
        signupUsername[0] = '\0';
        signupPassword[0] = '\0';
        signupConfirm[0] = '\0';
    }
};

struct AdminUiState {
    char newLeagueName[128];
    char newTeamName[128];
    int selectedLeagueForTeams;
    int selectedLeagueForMatches;
    int selectedLeagueForStandings;
    int selectedHomeTeam;
    int selectedAwayTeam;
    int homeScore;
    int awayScore;
    std::string errorMessage;
    std::string successMessage;

    AdminUiState() : selectedLeagueForTeams(0), selectedLeagueForMatches(0), selectedLeagueForStandings(0), selectedHomeTeam(0), selectedAwayTeam(0), homeScore(0), awayScore(0) {
        newLeagueName[0] = '\0';
        newTeamName[0] = '\0';
    }
};

struct ViewerUiState {
    int selectedLeagueIndex;
    std::string errorMessage;

    ViewerUiState() : selectedLeagueIndex(-1) {
    }
};

// Viewer screen helper.
// This stays read-only and reuses the same database read methods as the admin UI.
static void renderViewerScreen(Database& database, User* currentUser, ViewerUiState& viewerUi, bool& logoutRequested) {
    std::vector<League> leagues;
    try {
        leagues = database.getLeagues();
        if (viewerUi.selectedLeagueIndex >= static_cast<int>(leagues.size())) {
            viewerUi.selectedLeagueIndex = leagues.empty() ? -1 : 0;
        }
    } catch (const DatabaseException& e) {
        viewerUi.errorMessage = e.what();
    }

    ImGui::Begin("Viewer Dashboard", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::Text("Logged in as %s", currentUser->getUsername().c_str());
    ImGui::Text("Role: %s", currentUser->getRole().c_str());
    ImGui::SameLine(0.0f, 20.0f);

    if (ImGui::Button("Logout")) {
        logoutRequested = true;
    }

    if (!viewerUi.errorMessage.empty()) {
        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", viewerUi.errorMessage.c_str());
    }

    if (ImGui::BeginTabBar("ViewerTabs")) {
        if (ImGui::BeginTabItem("Browse Leagues")) {
            if (leagues.empty()) {
                ImGui::Text("No leagues available.");
            } else {
                ImGui::Text("Click a league to view it in the other tabs.");
                for (int i = 0; i < static_cast<int>(leagues.size()); ++i) {
                    bool selected = (viewerUi.selectedLeagueIndex == i);
                    if (ImGui::Selectable(leagues[i].getName().c_str(), selected)) {
                        viewerUi.selectedLeagueIndex = i;
                    }
                }
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Standings")) {
            if (viewerUi.selectedLeagueIndex < 0 || viewerUi.selectedLeagueIndex >= static_cast<int>(leagues.size())) {
                ImGui::Text("Select a league from Browse Leagues first.");
            } else {
                try {
                    int selectedLeagueId = leagues[viewerUi.selectedLeagueIndex].getId();
                    std::vector<Team> teams = database.getTeams(selectedLeagueId);
                    League league(leagues[viewerUi.selectedLeagueIndex].getName(), selectedLeagueId);

                    for (int i = 0; i < static_cast<int>(teams.size()); ++i) {
                        league.addTeam(teams[i]);
                    }

                    std::vector<Team> standings = league.getSortedStandings();

                    if (ImGui::BeginTable("viewerStandingsTable", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                        ImGui::TableSetupColumn("Rank");
                        ImGui::TableSetupColumn("Team");
                        ImGui::TableSetupColumn("P");
                        ImGui::TableSetupColumn("W");
                        ImGui::TableSetupColumn("D");
                        ImGui::TableSetupColumn("L");
                        ImGui::TableSetupColumn("GF");
                        ImGui::TableSetupColumn("GA");
                        ImGui::TableSetupColumn("GD");
                        ImGui::TableSetupColumn("Pts");
                        ImGui::TableHeadersRow();

                        for (int i = 0; i < static_cast<int>(standings.size()); ++i) {
                            ImGui::TableNextRow();
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", i + 1);
                            ImGui::TableNextColumn();
                            ImGui::Text("%s", standings[i].getName().c_str());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getPlayed());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getWins());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getDraws());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getLosses());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getGoalsFor());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getGoalsAgainst());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getGoalDifference());
                            ImGui::TableNextColumn();
                            ImGui::Text("%d", standings[i].getPoints());
                        }

                        ImGui::EndTable();
                    }
                } catch (const DatabaseException& e) {
                    viewerUi.errorMessage = e.what();
                    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", viewerUi.errorMessage.c_str());
                }
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Fixtures & Results")) {
            if (viewerUi.selectedLeagueIndex < 0 || viewerUi.selectedLeagueIndex >= static_cast<int>(leagues.size())) {
                ImGui::Text("Select a league from Browse Leagues first.");
            } else {
                try {
                    int selectedLeagueId = leagues[viewerUi.selectedLeagueIndex].getId();
                    std::vector<Match> matches = database.getMatches(selectedLeagueId);

                    if (matches.empty()) {
                        ImGui::Text("No fixtures or results yet.");
                    } else {
                        if (ImGui::BeginTable("viewerMatchesTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                            ImGui::TableSetupColumn("Home Team");
                            ImGui::TableSetupColumn("Score");
                            ImGui::TableSetupColumn("Away Team");
                            ImGui::TableSetupColumn("Date");
                            ImGui::TableHeadersRow();

                            for (int i = 0; i < static_cast<int>(matches.size()); ++i) {
                                ImGui::TableNextRow();
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", matches[i].getHomeTeamName().c_str());
                                ImGui::TableNextColumn();
                                ImGui::Text("%d - %d", matches[i].getHomeScore(), matches[i].getAwayScore());
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", matches[i].getAwayTeamName().c_str());
                                ImGui::TableNextColumn();
                                ImGui::Text("%s", matches[i].getMatchDate().c_str());
                            }

                            ImGui::EndTable();
                        }
                    }
                } catch (const DatabaseException& e) {
                    viewerUi.errorMessage = e.what();
                    ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", viewerUi.errorMessage.c_str());
                }
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    try {
        Database database;

        printf("Database initialized successfully. Checking tables...\n");
        const char* tables[] = {"leagues", "teams", "players", "matches", "users"};
        bool allTablesExist = true;

        FILE* proofFile = fopen("data/startup-check.txt", "w");
        if (proofFile != nullptr) {
            fprintf(proofFile, "Database initialized successfully.\n");
        }

        for (int i = 0; i < 5; ++i) {
            bool exists = database.tableExists(tables[i]);
            printf("  %s: %s\n", tables[i], exists ? "created" : "missing");
            if (!exists) {
                allTablesExist = false;
            }

            if (proofFile != nullptr) {
                fprintf(proofFile, "%s: %s\n", tables[i], exists ? "created" : "missing");
            }
        }

        if (allTablesExist) {
            printf("All required tables are present.\n");
            if (proofFile != nullptr) {
                fprintf(proofFile, "All required tables are present.\n");
            }
        } else {
            printf("One or more required tables are missing.\n");
            if (proofFile != nullptr) {
                fprintf(proofFile, "One or more required tables are missing.\n");
            }
        }

        if (proofFile != nullptr) {
            fclose(proofFile);
        }

        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return 1;

        const char* glsl_version = "#version 130";
        GLFWwindow* window = glfwCreateWindow(1280, 720, "Football Standings", nullptr, nullptr);
        if (window == nullptr)
            return 1;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        LoginState loginState;
        AdminUiState adminUi;
        ViewerUiState viewerUi;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (!loginState.loggedIn) {
                ImGui::Begin("Login", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                if (!loginState.showSignUp) {
                    ImGui::Text("Enter your username and password.");

                    if (!loginState.errorMessage.empty()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", loginState.errorMessage.c_str());
                    }

                    if (!loginState.signupSuccessMessage.empty()) {
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", loginState.signupSuccessMessage.c_str());
                    }

                    ImGui::InputText("Username", loginState.username, sizeof(loginState.username));
                    ImGui::InputText("Password", loginState.password, sizeof(loginState.password), ImGuiInputTextFlags_Password);

                    if (ImGui::Button("Login")) {
                        try {
                            if (loginState.currentUser != nullptr) {
                                delete loginState.currentUser;
                                loginState.currentUser = nullptr;
                            }

                            loginState.currentUser = database.authenticateUser(loginState.username, loginState.password);
                            loginState.loggedIn = true;
                            loginState.errorMessage.clear();
                            loginState.signupSuccessMessage.clear();
                            loginState.password[0] = '\0';
                        } catch (const DatabaseException& e) {
                            loginState.loggedIn = false;
                            loginState.errorMessage = e.what();

                            if (loginState.currentUser != nullptr) {
                                delete loginState.currentUser;
                                loginState.currentUser = nullptr;
                            }
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Sign Up")) {
                        loginState.showSignUp = true;
                        loginState.errorMessage.clear();
                        loginState.signupErrorMessage.clear();
                        loginState.signupSuccessMessage.clear();
                    }
                } else {
                    ImGui::Text("Create a new Viewer account.");

                    if (!loginState.signupErrorMessage.empty()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", loginState.signupErrorMessage.c_str());
                    }

                    ImGui::InputText("Username", loginState.signupUsername, sizeof(loginState.signupUsername));
                    ImGui::InputText("Password", loginState.signupPassword, sizeof(loginState.signupPassword), ImGuiInputTextFlags_Password);
                    ImGui::InputText("Confirm Password", loginState.signupConfirm, sizeof(loginState.signupConfirm), ImGuiInputTextFlags_Password);

                    if (ImGui::Button("Create Account")) {
                        if (std::string(loginState.signupPassword) != std::string(loginState.signupConfirm)) {
                            loginState.signupErrorMessage = "Passwords do not match";
                            loginState.signupSuccessMessage.clear();
                        } else {
                            try {
                                database.registerViewer(loginState.signupUsername, loginState.signupPassword);
                                loginState.signupErrorMessage.clear();
                                loginState.signupSuccessMessage = "Account created. Please log in.";

                                // Copy the new username back to the login form so the student can test it quickly.
                                std::snprintf(loginState.username, sizeof(loginState.username), "%s", loginState.signupUsername);
                                loginState.password[0] = '\0';

                                loginState.signupUsername[0] = '\0';
                                loginState.signupPassword[0] = '\0';
                                loginState.signupConfirm[0] = '\0';
                                loginState.showSignUp = false;
                            } catch (const DatabaseException& e) {
                                loginState.signupErrorMessage = e.what();
                                loginState.signupSuccessMessage.clear();
                            }
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Back to Login")) {
                        loginState.showSignUp = false;
                        loginState.signupErrorMessage.clear();
                        loginState.signupSuccessMessage.clear();
                    }
                }

                ImGui::End();
            } else if (loginState.currentUser->getRole() == "Admin") {
                ImGui::Begin("Admin Dashboard", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::Text("Logged in as %s", loginState.currentUser->getUsername().c_str());
                ImGui::Text("Role: %s", loginState.currentUser->getRole().c_str());
                ImGui::SameLine(0.0f, 20.0f);

                if (ImGui::Button("Logout")) {
                    delete loginState.currentUser;
                    loginState.currentUser = nullptr;
                    loginState.loggedIn = false;
                    loginState.showSignUp = false;
                    loginState.username[0] = '\0';
                    loginState.password[0] = '\0';
                    loginState.signupUsername[0] = '\0';
                    loginState.signupPassword[0] = '\0';
                    loginState.signupConfirm[0] = '\0';
                    loginState.errorMessage.clear();
                    loginState.signupErrorMessage.clear();
                    loginState.signupSuccessMessage.clear();
                    adminUi.errorMessage.clear();
                    adminUi.successMessage.clear();
                }

                if (loginState.currentUser->getRole() == "Admin") {
                    if (!adminUi.errorMessage.empty()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", adminUi.errorMessage.c_str());
                    }

                    if (!adminUi.successMessage.empty()) {
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", adminUi.successMessage.c_str());
                    }

                    std::vector<League> leagues = database.getLeagues();
                    std::vector<std::string> leagueNames;
                    std::vector<const char*> leagueLabels;
                    std::vector<int> leagueIds;

                    for (int i = 0; i < static_cast<int>(leagues.size()); ++i) {
                        leagueNames.push_back(leagues[i].getName());
                        leagueLabels.push_back(leagueNames[i].c_str());
                        leagueIds.push_back(leagues[i].getId());
                    }

                    if (ImGui::BeginTabBar("AdminTabs")) {
                        if (ImGui::BeginTabItem("Manage Leagues")) {
                            ImGui::InputText("New League Name", adminUi.newLeagueName, sizeof(adminUi.newLeagueName));
                            if (ImGui::Button("Create League")) {
                                try {
                                    database.createLeague(adminUi.newLeagueName);
                                    adminUi.newLeagueName[0] = '\0';
                                    adminUi.errorMessage.clear();
                                    adminUi.successMessage = "League created.";
                                } catch (const DatabaseException& e) {
                                    adminUi.errorMessage = e.what();
                                    adminUi.successMessage.clear();
                                }
                            }

                            if (leagues.empty()) {
                                ImGui::Text("No leagues yet.");
                            } else {
                                for (int i = 0; i < static_cast<int>(leagues.size()); ++i) {
                                    ImGui::Text("%s", leagues[i].getName().c_str());
                                    ImGui::SameLine();
                                    std::string deleteLabel = "Delete##league" + leagues[i].getName();
                                    if (ImGui::Button(deleteLabel.c_str())) {
                                        try {
                                            database.deleteLeague(leagues[i].getId());
                                            adminUi.errorMessage.clear();
                                            adminUi.successMessage = "League deleted.";
                                        } catch (const DatabaseException& e) {
                                            adminUi.errorMessage = e.what();
                                            adminUi.successMessage.clear();
                                        }
                                    }
                                }
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Manage Teams")) {
                            if (!leagueLabels.empty()) {
                                if (adminUi.selectedLeagueForTeams >= static_cast<int>(leagueLabels.size())) {
                                    adminUi.selectedLeagueForTeams = 0;
                                }
                                ImGui::Combo("League", &adminUi.selectedLeagueForTeams, leagueLabels.data(), static_cast<int>(leagueLabels.size()));
                            }

                            ImGui::InputText("New Team Name", adminUi.newTeamName, sizeof(adminUi.newTeamName));
                            if (ImGui::Button("Add Team")) {
                                try {
                                    int selectedLeagueId = leagueIds[adminUi.selectedLeagueForTeams];
                                    database.addTeam(selectedLeagueId, adminUi.newTeamName);
                                    adminUi.newTeamName[0] = '\0';
                                    adminUi.errorMessage.clear();
                                    adminUi.successMessage = "Team added.";
                                } catch (const DatabaseException& e) {
                                    adminUi.errorMessage = e.what();
                                    adminUi.successMessage.clear();
                                }
                            }

                            if (!leagueLabels.empty()) {
                                int selectedLeagueId = leagueIds[adminUi.selectedLeagueForTeams];
                                std::vector<Team> teams = database.getTeams(selectedLeagueId);

                                if (ImGui::BeginTable("teamTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                                    ImGui::TableSetupColumn("Team");
                                    ImGui::TableSetupColumn("Action");
                                    ImGui::TableHeadersRow();

                                    for (int i = 0; i < static_cast<int>(teams.size()); ++i) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%s", teams[i].getName().c_str());
                                        ImGui::TableNextColumn();
                                        std::string deleteLabel = "Delete##team" + teams[i].getName();
                                        if (ImGui::Button(deleteLabel.c_str())) {
                                            try {
                                                database.deleteTeam(teams[i].getId());
                                                adminUi.errorMessage.clear();
                                                adminUi.successMessage = "Team deleted.";
                                            } catch (const DatabaseException& e) {
                                                adminUi.errorMessage = e.what();
                                                adminUi.successMessage.clear();
                                            }
                                        }
                                    }

                                    ImGui::EndTable();
                                }
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Enter Match Result")) {
                            if (!leagueLabels.empty()) {
                                if (adminUi.selectedLeagueForMatches >= static_cast<int>(leagueLabels.size())) {
                                    adminUi.selectedLeagueForMatches = 0;
                                }
                                ImGui::Combo("League", &adminUi.selectedLeagueForMatches, leagueLabels.data(), static_cast<int>(leagueLabels.size()));

                                int selectedLeagueId = leagueIds[adminUi.selectedLeagueForMatches];
                                std::vector<Team> teams = database.getTeams(selectedLeagueId);
                                std::vector<std::string> teamNames;
                                std::vector<const char*> teamLabels;
                                for (int i = 0; i < static_cast<int>(teams.size()); ++i) {
                                    teamNames.push_back(teams[i].getName());
                                    teamLabels.push_back(teamNames[i].c_str());
                                }

                                if (!teamLabels.empty()) {
                                    if (adminUi.selectedHomeTeam >= static_cast<int>(teamLabels.size())) {
                                        adminUi.selectedHomeTeam = 0;
                                    }
                                    if (adminUi.selectedAwayTeam >= static_cast<int>(teamLabels.size())) {
                                        adminUi.selectedAwayTeam = 0;
                                    }

                                    ImGui::Combo("Home Team", &adminUi.selectedHomeTeam, teamLabels.data(), static_cast<int>(teamLabels.size()));
                                    ImGui::Combo("Away Team", &adminUi.selectedAwayTeam, teamLabels.data(), static_cast<int>(teamLabels.size()));
                                }

                                ImGui::InputInt("Home Score", &adminUi.homeScore);
                                ImGui::InputInt("Away Score", &adminUi.awayScore);

                                if (ImGui::Button("Submit Result")) {
                                    try {
                                        int homeTeamId = teams[adminUi.selectedHomeTeam].getId();
                                        int awayTeamId = teams[adminUi.selectedAwayTeam].getId();
                                        database.recordMatchResult(selectedLeagueId, homeTeamId, awayTeamId, adminUi.homeScore, adminUi.awayScore, "2026-08-09");
                                        adminUi.errorMessage.clear();
                                        adminUi.successMessage = "Match result saved.";
                                        adminUi.homeScore = 0;
                                        adminUi.awayScore = 0;
                                    } catch (const DatabaseException& e) {
                                        adminUi.errorMessage = e.what();
                                        adminUi.successMessage.clear();
                                    }
                                }
                            } else {
                                ImGui::Text("Create a league first.");
                            }

                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("View Standings")) {
                            if (!leagueLabels.empty()) {
                                if (adminUi.selectedLeagueForStandings >= static_cast<int>(leagueLabels.size())) {
                                    adminUi.selectedLeagueForStandings = 0;
                                }
                                ImGui::Combo("League", &adminUi.selectedLeagueForStandings, leagueLabels.data(), static_cast<int>(leagueLabels.size()));

                                int selectedLeagueId = leagueIds[adminUi.selectedLeagueForStandings];
                                std::vector<Team> teams = database.getTeams(selectedLeagueId);
                                League league(leagues[adminUi.selectedLeagueForStandings].getName(), leagues[adminUi.selectedLeagueForStandings].getId());

                                for (int i = 0; i < static_cast<int>(teams.size()); ++i) {
                                    league.addTeam(teams[i]);
                                }

                                std::vector<Team> standings = league.getSortedStandings();

                                if (ImGui::BeginTable("standingsTable", 10, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
                                    ImGui::TableSetupColumn("Rank");
                                    ImGui::TableSetupColumn("Team");
                                    ImGui::TableSetupColumn("P");
                                    ImGui::TableSetupColumn("W");
                                    ImGui::TableSetupColumn("D");
                                    ImGui::TableSetupColumn("L");
                                    ImGui::TableSetupColumn("GF");
                                    ImGui::TableSetupColumn("GA");
                                    ImGui::TableSetupColumn("GD");
                                    ImGui::TableSetupColumn("Pts");
                                    ImGui::TableHeadersRow();

                                    for (int i = 0; i < static_cast<int>(standings.size()); ++i) {
                                        ImGui::TableNextRow();
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", i + 1);
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%s", standings[i].getName().c_str());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getPlayed());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getWins());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getDraws());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getLosses());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getGoalsFor());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getGoalsAgainst());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getGoalDifference());
                                        ImGui::TableNextColumn();
                                        ImGui::Text("%d", standings[i].getPoints());
                                    }

                                    ImGui::EndTable();
                                }
                            } else {
                                ImGui::Text("Create a league first.");
                            }

                            ImGui::EndTabItem();
                        }

                        ImGui::EndTabBar();
                    }
                }

                ImGui::End();
            } else if (loginState.currentUser->getRole() == "Viewer") {
                bool viewerLogoutRequested = false;
                renderViewerScreen(database, loginState.currentUser, viewerUi, viewerLogoutRequested);

                if (viewerLogoutRequested) {
                    delete loginState.currentUser;
                    loginState.currentUser = nullptr;
                    loginState.loggedIn = false;
                    loginState.showSignUp = false;
                    loginState.username[0] = '\0';
                    loginState.password[0] = '\0';
                    loginState.signupUsername[0] = '\0';
                    loginState.signupPassword[0] = '\0';
                    loginState.signupConfirm[0] = '\0';
                    loginState.errorMessage.clear();
                    loginState.signupErrorMessage.clear();
                    loginState.signupSuccessMessage.clear();
                    viewerUi.errorMessage.clear();
                    viewerUi.selectedLeagueIndex = -1;
                }
            } else {
                ImGui::Begin("Dashboard", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                ImGui::Text("Unknown role.");
                ImGui::End();
            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        if (loginState.currentUser != nullptr) {
            delete loginState.currentUser;
        }

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    } catch (const DatabaseException& e) {
        fprintf(stderr, "Database error: %s\n", e.what());
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }

    return 1;
}