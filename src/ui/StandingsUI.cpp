#include "StandingsUI.h"
#include "DesignSystem.h"
#include "imgui.h"

using namespace UI;

void StandingsUI::render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache, ViewerUiState& viewerUi) {
    Design::PageTitle("My League Standings");

    auto leagues = uiCache.getLeagues();
    if (viewerUi.selectedLeagueIndex < 0 || viewerUi.selectedLeagueIndex >= static_cast<int>(leagues.size())) {
        Design::EmptyState("Select a league first.");
        if (!leagues.empty()) {
            if (Design::PrimaryButton("Go to My Leagues")) appState.currentPage = Page::CustomLeagues;
        }
        return;
    }

    League league(leagues[viewerUi.selectedLeagueIndex].getName(), leagues[viewerUi.selectedLeagueIndex].getId());
    int lid = league.getId();

    Design::TextMuted(("League: " + league.getName()).c_str());
    Design::Spacing(2);

    auto teams = uiCache.getTeams(lid);
    if (teams.empty()) {
        dbWorker.postTaskOnce("refresh_teams_" + std::to_string(lid), [lid, &uiCache](Database& db){ uiCache.refreshTeams(db, lid); });
        Design::EmptyState("Loading teams...");
        return;
    }

    for (const auto& t : teams) league.addTeam(t);
    auto standings = league.getSortedStandings();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Design::ColSurface);
    ImGui::PushStyleColor(ImGuiCol_Border, Design::ColBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    
    if (ImGui::BeginChild("custom_standings_panel", ImVec2(0, 0), true)) {
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1,1,1,0.05f));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 8));

        if (ImGui::BeginTable("points_table", 7, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerH)) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("Team", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("P", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("D", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("Pts", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            
            ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextSecondary);
            ImGui::TableHeadersRow();
            ImGui::PopStyleColor();

            for (int i = 0; i < static_cast<int>(standings.size()); ++i) {
                const Team& t = standings[i];
                ImGui::PushID(i);
                ImGui::TableNextRow();
                ImGui::TableNextColumn(); Design::TextLabel(std::to_string(i+1).c_str());
                ImGui::TableNextColumn(); ImGui::Text("%s", t.getName().c_str());
                ImGui::TableNextColumn(); ImGui::Text("%d", t.getPlayed());
                ImGui::TableNextColumn(); ImGui::Text("%d", t.getWins());
                ImGui::TableNextColumn(); ImGui::Text("%d", t.getDraws());
                ImGui::TableNextColumn(); ImGui::Text("%d", t.getLosses());
                ImGui::TableNextColumn(); 
                ImGui::PushStyleColor(ImGuiCol_Text, Design::ColPrimary);
                ImGui::Text("%d", t.getPoints());
                ImGui::PopStyleColor();
                ImGui::PopID();
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
