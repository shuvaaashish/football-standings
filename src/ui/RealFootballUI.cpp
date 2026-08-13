#include "RealFootballUI.h"
#include "DesignSystem.h"
#include "imgui.h"
#include <string>
#include <map>
#include <vector>

using namespace UI;

// ─── Competitions Page ──────────────────────────────────────────────────────

void RealFootballUI::renderCompetitions(FootballApi::RealFootballService& service) {
    Design::PageTitle("Real Football Competitions");

    // Error display
    if (!service.getErrorMessage().empty()) {
        ImGui::TextColored(Design::ColError, "API Error: %s", service.getErrorMessage().c_str());
        Design::Spacing();
    }

    if (!service.areCompetitionsLoaded()) {
        Design::EmptyState("Competitions have not been loaded yet.");
        if (Design::PrimaryButton("Load Competitions")) {
            service.loadCompetitions();
        }
        return;
    }

    const auto& comps = service.getCompetitions();
    Design::TextMuted((std::to_string(comps.size()) + " competitions available. Select one to browse matches and standings.").c_str());
    Design::Spacing(2);

    int selectedId = service.getSelectedCompetitionId();
    float avail = ImGui::GetContentRegionAvail().x;
    int cols = std::max(1, static_cast<int>(avail / 280.0f));
    ImGui::Columns(cols, nullptr, false);

    for (const auto& c : comps) {
        if (c.code.empty()) continue;

        bool isSelected = (c.id == selectedId);
        
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Border, Design::ColPrimary);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 2.0f);
        }

        if (Design::BeginCard(("comp_" + std::to_string(c.id)).c_str(), ImVec2(0, 100))) {
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, Design::ColPrimary);
                ImGui::Text("%s", c.name.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::Text("%s", c.name.c_str());
            }

            Design::TextLabel(c.areaName.c_str());
            
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 28.0f);
            
            if (isSelected) {
                Design::Badge("SELECTED", Design::ColPrimary);
            } else {
                if (ImGui::Button(("Select##" + std::to_string(c.id)).c_str(), ImVec2(-1, 0))) {
                    service.selectCompetition(c.id, c.code);
                }
            }
            Design::EndCard();
        }
        
        if (isSelected) {
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
        }

        ImGui::NextColumn();
    }
    ImGui::Columns(1);
}

// ─── Matches Page ───────────────────────────────────────────────────────────

void RealFootballUI::renderMatches(FootballApi::RealFootballService& service) {
    Design::PageTitle("Real Football Matches");

    // Error display
    if (!service.getErrorMessage().empty()) {
        ImGui::TextColored(Design::ColError, "API Error: %s", service.getErrorMessage().c_str());
        Design::Spacing();
    }

    if (service.getSelectedCompetitionId() == 0) {
        Design::EmptyState("No competition selected. Go to Competitions and select one first.");
        return;
    }

    const auto& comps = service.getCompetitions();
    std::string compName = "Unknown";
    for (const auto& c : comps) {
        if (c.id == service.getSelectedCompetitionId()) {
            compName = c.name;
            break;
        }
    }
    
    Design::TextMuted(("Competition: " + compName).c_str());
    Design::Spacing(2);

    if (!service.areMatchesLoaded()) {
        Design::EmptyState("Matches have not been loaded yet.");
        if (Design::PrimaryButton("Load Matches")) {
            service.loadMatches();
        }
        return;
    }

    const auto& matches = service.getMatches();
    
    // Group matches by date
    std::map<std::string, std::vector<RealFootball::RealMatch>> byDate;
    for (const auto& m : matches) {
        std::string date = m.utcDate.substr(0, 10);
        byDate[date].push_back(m);
    }
    
    for (auto it = byDate.rbegin(); it != byDate.rend(); ++it) {
        Design::SectionTitle(it->first.c_str());
        int rowIndex = 0;
        for (const auto& m : it->second) {
            ImGui::PushID(m.id != 0 ? m.id : rowIndex);
            bool hasScore = (m.status == "FINISHED" || m.status == "IN_PLAY" || m.status == "PAUSED");
            int hScore = m.homeScoreFullTime.value_or(0);
            int aScore = m.awayScoreFullTime.value_or(0);
            
            Design::MatchRow(m.homeTeam.name, hScore, aScore, m.awayTeam.name, m.status, m.utcDate, hasScore);
            ImGui::PopID();
            ++rowIndex;
        }
        Design::Spacing(2);
    }
}

// ─── Standings Page ─────────────────────────────────────────────────────────

void RealFootballUI::renderStandings(FootballApi::RealFootballService& service) {
    Design::PageTitle("Real Football Standings");

    // Error display
    if (!service.getErrorMessage().empty()) {
        ImGui::TextColored(Design::ColError, "API Error: %s", service.getErrorMessage().c_str());
        Design::Spacing();
    }

    if (service.getSelectedCompetitionId() == 0) {
        Design::EmptyState("No competition selected. Go to Competitions and select one first.");
        return;
    }

    const auto& comps = service.getCompetitions();
    std::string compName = "Unknown";
    for (const auto& c : comps) {
        if (c.id == service.getSelectedCompetitionId()) {
            compName = c.name;
            break;
        }
    }
    
    Design::TextMuted(("Competition: " + compName).c_str());
    Design::Spacing(2);

    if (!service.areStandingsLoaded()) {
        Design::EmptyState("Standings have not been loaded yet.");
        if (Design::PrimaryButton("Load Standings")) {
            service.loadStandings();
        }
        return;
    }

    const auto& standings = service.getStandings();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Design::ColSurface);
    ImGui::PushStyleColor(ImGuiCol_Border, Design::ColBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    
    if (ImGui::BeginChild("standings_panel", ImVec2(0, 0), true)) {
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1,1,1,0.05f));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 8));
        
        if (ImGui::BeginTable("real_standings_table", 10, ImGuiTableFlags_RowBg | ImGuiTableFlags_ScrollY | ImGuiTableFlags_BordersInnerH, ImVec2(0, 0))) {
            ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("Team", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("P", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("W", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("D", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("L", ImGuiTableColumnFlags_WidthFixed, 30.0f);
            ImGui::TableSetupColumn("GF", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("GA", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("GD", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            ImGui::TableSetupColumn("Pts", ImGuiTableColumnFlags_WidthFixed, 35.0f);
            
            // Header styling
            ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextSecondary);
            ImGui::TableHeadersRow();
            ImGui::PopStyleColor();

            for (size_t i = 0; i < standings.size(); ++i) {
                const auto& s = standings[i];
                ImGui::PushID(static_cast<int>(i));
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                Design::TextLabel(std::to_string(s.position).c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%s", s.team.name.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.playedGames);

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.won);

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.draw);

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.lost);

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.goalsFor);

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.goalsAgainst);

                ImGui::TableNextColumn();
                ImGui::Text("%d", s.goalDifference);

                ImGui::TableNextColumn();
                ImGui::PushStyleColor(ImGuiCol_Text, Design::ColPrimary);
                ImGui::Text("%d", s.points);
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
