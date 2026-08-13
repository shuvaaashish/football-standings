#include "DashboardUI.h"
#include "DesignSystem.h"
#include "imgui.h"
#include <algorithm>

using namespace UI;

void DashboardUI::render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache,
                         FootballApi::RealFootballService& realFootballService,
                         ViewerUiState& viewerUi, User* currentUser) {
    
    Design::PageTitle("Football Dashboard");
    Design::TextMuted("Browse real competitions, follow matches, and manage your leagues.");
    Design::Spacing(3);

    // ── Quick Stats ──
    Design::SectionTitle("Overview");
    
    const auto& leagues = uiCache.getLeagues();
    if (leagues.empty()) {
        dbWorker.postTaskOnce("refresh_leagues", [&uiCache](Database& db){ uiCache.refreshLeagues(db); });
    }

    if (ImGui::BeginChild("stats_row", ImVec2(0, 110), false)) {
        float cardWidth = 240.0f;

        // Card 1: Selected Competition
        if (Design::BeginCard("stat_selected", ImVec2(cardWidth, 90))) {
            Design::TextLabel("Selected Competition");
            Design::Spacing(1);
            if (realFootballService.getSelectedCompetitionId() != 0) {
                std::string compName = "Unknown";
                for (const auto& c : realFootballService.getCompetitions()) {
                    if (c.id == realFootballService.getSelectedCompetitionId()) {
                        compName = c.name;
                        break;
                    }
                }
                ImGui::PushStyleColor(ImGuiCol_Text, Design::ColPrimary);
                ImGui::SetWindowFontScale(1.1f);
                ImGui::Text("%s", compName.c_str());
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();
            } else {
                Design::TextMuted("None selected");
            }
            Design::EndCard();
        }

        ImGui::SameLine();

        // Card 2: Real Competitions
        if (Design::BeginCard("stat_real", ImVec2(cardWidth, 90))) {
            Design::TextLabel("Real Competitions");
            Design::Spacing(1);
            if (realFootballService.areCompetitionsLoaded()) {
                ImGui::PushStyleColor(ImGuiCol_Text, Design::ColSuccess);
                ImGui::SetWindowFontScale(1.1f);
                ImGui::Text("%d loaded", static_cast<int>(realFootballService.getCompetitions().size()));
                ImGui::SetWindowFontScale(1.0f);
                ImGui::PopStyleColor();
            } else {
                Design::TextMuted("Not loaded");
            }
            Design::EndCard();
        }

        ImGui::SameLine();

        // Card 3: Custom Leagues
        if (Design::BeginCard("stat_custom", ImVec2(cardWidth, 90))) {
            Design::TextLabel("My Leagues");
            Design::Spacing(1);
            ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextPrimary);
            ImGui::SetWindowFontScale(1.1f);
            ImGui::Text("%d", static_cast<int>(leagues.size()));
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            Design::EndCard();
        }
    }
    ImGui::EndChild();

    Design::Spacing(2);

    // ── Quick Access ──
    Design::SectionTitle("Quick Access");
    if (ImGui::BeginChild("quick_access", ImVec2(0, 135), false)) {
        float cardWidth = 240.0f;
        
        if (Design::BeginCard("qa_real", ImVec2(cardWidth, 110))) {
            ImGui::Text("Browse Competitions");
            ImGui::SetCursorPosY(56.0f);
            if (Design::PrimaryButton("Open##1", ImVec2(-1, 0))) {
                appState.currentPage = Page::RealCompetitions;
            }
            Design::EndCard();
        }
        
        ImGui::SameLine();
        
        if (Design::BeginCard("qa_custom", ImVec2(cardWidth, 110))) {
            ImGui::Text("View My Leagues");
            ImGui::SetCursorPosY(56.0f);
            if (Design::PrimaryButton("Open##2", ImVec2(-1, 0))) {
                appState.currentPage = Page::CustomLeagues;
            }
            Design::EndCard();
        }
        
        if (appState.role == "Admin") {
            ImGui::SameLine();
            if (Design::BeginCard("qa_admin", ImVec2(cardWidth, 110))) {
                ImGui::Text("Management");
                ImGui::SetCursorPosY(56.0f);
                if (Design::PrimaryButton("Open##3", ImVec2(-1, 0))) {
                    appState.currentPage = Page::Admin;
                }
                Design::EndCard();
            }
        }
    }
    ImGui::EndChild();

    Design::Spacing(2);

    // ── Recent Results ──
    Design::SectionTitle("Recent Custom Results");

    std::vector<Match> recent;
    for (const auto& L : leagues) {
        auto m = uiCache.getMatches(L.getId());
        if (m.empty()) dbWorker.postTaskOnce("refresh_matches_" + std::to_string(L.getId()), [id=L.getId(), &uiCache](Database& db){ uiCache.refreshMatches(db, id); });
        recent.insert(recent.end(), m.begin(), m.end());
    }

    std::sort(recent.begin(), recent.end(), [](const Match& a, const Match& b){ return a.getMatchDate() > b.getMatchDate(); });

    if (recent.empty()) {
        Design::EmptyState("No match results yet.");
    } else {
        int shown = 0;
        for (const auto& mt : recent) {
            if (shown++ > 5) break;
            Design::MatchRow(mt.getHomeTeamName(), mt.getHomeScore(), mt.getAwayScore(), mt.getAwayTeamName(), "FINISHED", mt.getMatchDate(), true);
        }
    }
}
