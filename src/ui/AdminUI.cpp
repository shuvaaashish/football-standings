#include "AdminUI.h"
#include "DesignSystem.h"
#include "imgui.h"
#include <string>
#include <vector>

using namespace UI;

void AdminUI::render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache, AdminUiState& adminUi, User* currentUser) {
    Design::PageTitle("Admin Management");

    if (!adminUi.errorMessage.empty()) {
        ImGui::TextColored(Design::ColError, "%s", adminUi.errorMessage.c_str());
        Design::Spacing();
    }
    if (!adminUi.successMessage.empty()) {
        ImGui::TextColored(Design::ColSuccess, "%s", adminUi.successMessage.c_str());
        Design::Spacing();
    }

    auto leagues = uiCache.getLeagues();
    if (leagues.empty()) dbWorker.postTaskOnce("refresh_leagues", [&uiCache](Database& db){ uiCache.refreshLeagues(db); });

    std::vector<std::string> leagueNames;
    std::vector<const char*> leagueLabels;
    std::vector<int> leagueIds;
    leagueNames.reserve(leagues.size());
    leagueLabels.reserve(leagues.size());
    leagueIds.reserve(leagues.size());
    for (const auto& L : leagues) {
        leagueNames.push_back(L.getName());
        leagueIds.push_back(L.getId());
    }
    for (const auto& name : leagueNames) {
        leagueLabels.push_back(name.c_str());
    }

    if (ImGui::BeginTabBar("AdminTabs")) {
        if (ImGui::BeginTabItem("Dashboard")) {
            Design::Spacing(2);
            Design::SectionTitle("System Statistics");
            if (Design::BeginCard("admin_stats", ImVec2(0, 100))) {
                ImGui::Text("%d Total Leagues", static_cast<int>(leagues.size()));
                int totalTeams = 0; for (int id : leagueIds) totalTeams += static_cast<int>(uiCache.getTeams(id).size());
                ImGui::Text("%d Total Teams", totalTeams);
                int totalMatches = 0; for (int id : leagueIds) totalMatches += static_cast<int>(uiCache.getMatches(id).size());
                ImGui::Text("%d Total Matches", totalMatches);
                Design::EndCard();
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Manage Leagues")) {
            Design::Spacing(2);
            Design::SectionTitle("Create New League");
            if (Design::BeginCard("create_league_card", ImVec2(0, 100))) {
                ImGui::InputText("##NewLeague", adminUi.newLeagueName, sizeof(adminUi.newLeagueName));
                ImGui::SameLine();
                if (Design::PrimaryButton("Create League", ImVec2(120, 0))) {
                    std::string name(adminUi.newLeagueName);
                    auto result = std::make_shared<std::string>();
                    dbWorker.postTask([name, result, &uiCache](Database& db){ try{ db.createLeague(name); uiCache.refreshLeagues(db);} catch(const DatabaseException& e){ *result = e.what(); } },
                                      [result, &adminUi](){ if (result->empty()) { adminUi.successMessage = "League created."; adminUi.errorMessage.clear(); } else { adminUi.errorMessage = *result; adminUi.successMessage.clear(); } });
                    adminUi.newLeagueName[0] = '\0';
                }
                Design::EndCard();
            }

            Design::Spacing(2);
            Design::SectionTitle("Existing Leagues");
            if (leagues.empty()) {
                Design::EmptyState("No leagues yet.");
            } else {
                for (size_t i=0;i<leagues.size();++i) {
                    if (Design::BeginCard((std::string("ex_league_") + std::to_string(leagues[i].getId())).c_str(), ImVec2(0, 60))) {
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8.0f);
                        ImGui::Text("%s", leagues[i].getName().c_str());
                        ImGui::SameLine(ImGui::GetContentRegionAvail().x - 100.0f);
                        std::string del = "Delete##l" + std::to_string(leagues[i].getId());
                        if (ImGui::Button(del.c_str(), ImVec2(80, 0))) {
                            int id = leagues[i].getId();
                            auto result = std::make_shared<std::string>();
                            dbWorker.postTask([id, result, &uiCache](Database& db){ try{ db.deleteLeague(id); uiCache.refreshLeagues(db);} catch(const DatabaseException& e){ *result = e.what(); } },
                                              [result, &adminUi](){ if (result->empty()) { adminUi.successMessage = "League deleted."; adminUi.errorMessage.clear(); } else { adminUi.errorMessage = *result; adminUi.successMessage.clear(); } });
                        }
                        Design::EndCard();
                    }
                }
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Manage Teams")) {
            Design::Spacing(2);
            Design::SectionTitle("Add Team to League");
            
            if (!leagueLabels.empty()) {
                if (Design::BeginCard("add_team_card", ImVec2(0, 140))) {
                    if (adminUi.selectedLeagueForTeams >= static_cast<int>(leagueLabels.size())) adminUi.selectedLeagueForTeams = 0;
                    ImGui::Combo("League##Team", &adminUi.selectedLeagueForTeams, leagueLabels.data(), static_cast<int>(leagueLabels.size()));
                    Design::Spacing();
                    
                    ImGui::InputText("##NewTeam", adminUi.newTeamName, sizeof(adminUi.newTeamName));
                    ImGui::SameLine();
                    if (Design::PrimaryButton("Add Team", ImVec2(120, 0))) {
                        if (leagueIds.empty()) { adminUi.errorMessage = "No league selected."; }
                        else {
                            int lid = leagueIds[adminUi.selectedLeagueForTeams];
                            std::string name(adminUi.newTeamName);
                            auto result = std::make_shared<std::string>();
                            dbWorker.postTask([lid, name, result, &uiCache](Database& db){ try{ db.addTeam(lid, name); uiCache.refreshTeams(db,lid);} catch(const DatabaseException& e){ *result = e.what(); } },
                                              [result, &adminUi](){ if (result->empty()) { adminUi.successMessage = "Team added."; adminUi.errorMessage.clear(); } else { adminUi.errorMessage = *result; adminUi.successMessage.clear(); } });
                            adminUi.newTeamName[0] = '\0';
                        }
                    }
                    Design::EndCard();
                }

                Design::Spacing(2);
                Design::SectionTitle("Teams in Selected League");
                int lid = leagueIds[adminUi.selectedLeagueForTeams];
                auto teams = uiCache.getTeams(lid);
                if (teams.empty()) dbWorker.postTaskOnce("refresh_teams_" + std::to_string(lid), [lid, &uiCache](Database& db){ uiCache.refreshTeams(db,lid); });

                if (teams.empty()) {
                    Design::EmptyState("No teams in this league.");
                } else {
                    ImGui::PushStyleColor(ImGuiCol_ChildBg, Design::ColSurface);
                    ImGui::PushStyleColor(ImGuiCol_Border, Design::ColBorder);
                    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
                    
                    if (ImGui::BeginChild("admin_teams_table_panel", ImVec2(0, 0), true)) {
                        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1,1,1,0.05f));
                        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 8));
                        if (ImGui::BeginTable("teamTableAdmin", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerH)) {
                            ImGui::TableSetupColumn("Team", ImGuiTableColumnFlags_WidthStretch); 
                            ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 100.0f); 
                            ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextSecondary);
                            ImGui::TableHeadersRow();
                            ImGui::PopStyleColor();
                            for (size_t i=0;i<teams.size();++i) {
                                ImGui::TableNextRow(); 
                                ImGui::TableNextColumn(); ImGui::Text("%s", teams[i].getName().c_str()); 
                                ImGui::TableNextColumn();
                                std::string del = "Delete##t" + std::to_string(teams[i].getId());
                                if (ImGui::Button(del.c_str(), ImVec2(80, 0))) {
                                    int tid = teams[i].getId();
                                    auto result = std::make_shared<std::string>();
                                    dbWorker.postTask([tid, result, &uiCache](Database& db){ try{ db.deleteTeam(tid); uiCache.refreshLeagues(db);} catch(const DatabaseException& e){ *result = e.what(); } },
                                                      [result, &adminUi](){ if (result->empty()) { adminUi.successMessage = "Team deleted."; adminUi.errorMessage.clear(); } else { adminUi.errorMessage = *result; adminUi.successMessage.clear(); } });
                                }
                            }
                            ImGui::EndTable();
                        }
                        ImGui::PopStyleVar();
                        ImGui::PopStyleColor();
                    }
                    ImGui::EndChild();
                    ImGui::PopStyleVar();
                    ImGui::PopStyleColor(2);
                }
            } else {
                Design::EmptyState("No leagues available. Create one first.");
            }
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Enter Match Result")) {
            Design::Spacing(2);
            Design::SectionTitle("New Result");
            
            if (!leagueLabels.empty()) {
                if (Design::BeginCard("enter_result_card", ImVec2(0, 300))) {
                    if (adminUi.selectedLeagueForMatches >= static_cast<int>(leagueLabels.size())) adminUi.selectedLeagueForMatches = 0;
                    ImGui::Combo("League##Match", &adminUi.selectedLeagueForMatches, leagueLabels.data(), static_cast<int>(leagueLabels.size()));

                    int selectedLeagueId = leagueIds[adminUi.selectedLeagueForMatches];
                    auto teams = uiCache.getTeams(selectedLeagueId);
                    if (teams.empty()) dbWorker.postTaskOnce("refresh_teams_" + std::to_string(selectedLeagueId), [selectedLeagueId, &uiCache](Database& db){ uiCache.refreshTeams(db, selectedLeagueId); });

                    std::vector<std::string> teamNames;
                    std::vector<const char*> teamLabels;
                    for (const auto& t : teams) teamNames.push_back(t.getName());
                    for (const auto& name : teamNames) teamLabels.push_back(name.c_str());

                    Design::Spacing();

                    if (!teamLabels.empty()) {
                        if (adminUi.selectedHomeTeam >= static_cast<int>(teamLabels.size())) adminUi.selectedHomeTeam = 0;
                        if (adminUi.selectedAwayTeam >= static_cast<int>(teamLabels.size())) adminUi.selectedAwayTeam = 0;
                        ImGui::Combo("Home Team", &adminUi.selectedHomeTeam, teamLabels.data(), static_cast<int>(teamLabels.size()));
                        ImGui::Combo("Away Team", &adminUi.selectedAwayTeam, teamLabels.data(), static_cast<int>(teamLabels.size()));
                    }

                    Design::Spacing();
                    ImGui::InputInt("Home Score", &adminUi.homeScore);
                    ImGui::InputInt("Away Score", &adminUi.awayScore);
                    ImGui::InputText("Match Date (YYYY-MM-DD)", adminUi.matchDateBuffer, sizeof(adminUi.matchDateBuffer));
                    Design::Spacing();

                    if (Design::PrimaryButton("Submit Result", ImVec2(150, 40))) {
                        if (teams.empty()) { adminUi.errorMessage = "No teams available for selected league."; }
                        else if (adminUi.matchDateBuffer[0] == '\0') {
                            adminUi.errorMessage = "Please enter a match date (YYYY-MM-DD).";
                            adminUi.successMessage.clear();
                        }
                        else {
                            int homeTeamId = teams[adminUi.selectedHomeTeam].getId();
                            int awayTeamId = teams[adminUi.selectedAwayTeam].getId();
                            int homeScore = adminUi.homeScore;
                            int awayScore = adminUi.awayScore;
                            int leagueId = selectedLeagueId;
                            std::string matchDate = adminUi.matchDateBuffer;

                            auto result = std::make_shared<std::string>();
                            dbWorker.postTask([leagueId, homeTeamId, awayTeamId, homeScore, awayScore, matchDate, result, &uiCache](Database& db){ try{ db.recordMatchResult(leagueId, homeTeamId, awayTeamId, homeScore, awayScore, matchDate); uiCache.refreshTeams(db, leagueId); uiCache.refreshMatches(db, leagueId);} catch(const DatabaseException& e){ *result = e.what(); } },
                                              [result, &adminUi](){ if (result->empty()) { adminUi.successMessage = "Match result saved."; adminUi.errorMessage.clear(); adminUi.homeScore=0; adminUi.awayScore=0; adminUi.matchDateBuffer[0] = '\0';} else { adminUi.errorMessage = *result; adminUi.successMessage.clear(); } });
                        }
                    }
                    Design::EndCard();
                }
            } else {
                Design::EmptyState("Create a league first.");
            }

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}
