#include "RealFootballUI.h"
#include "DesignSystem.h"
#include "ImageCache.h"
#include "imgui.h"
#include <string>
#include <map>
#include <vector>
#include <ctime>
#include <iomanip>
#include <sstream>

using namespace UI;

namespace {

UI::ImageCache& GetRealLogoCache() {
    static UI::ImageCache cache;
    return cache;
}

} // namespace

// ─── Competitions Page ──────────────────────────────────────────────────────

void RealFootballUI::renderCompetitions(FootballApi::RealFootballService& service) {
    Design::PageTitle("Real Football Competitions");

    // Error display
    if (!service.getErrorMessage().empty()) {
        ImGui::TextColored(Design::ColError, "API Error: %s", service.getErrorMessage().c_str());
        Design::Spacing();
    }

    if (!service.areCompetitionsLoaded()) {
        if (service.isLoadingCompetitions()) {
            Design::EmptyState("Loading competitions...");
        } else {
            Design::EmptyState("Competitions have not been loaded yet.");
            if (Design::PrimaryButton("Load Competitions")) service.loadCompetitions();
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

        if (Design::BeginCard(("comp_" + std::to_string(c.id)).c_str(), ImVec2(0, 120))) {
            ImGui::BeginGroup();
            if (!c.emblemUrl.empty()) {
                ImTextureID tex = GetRealLogoCache().getTexture(c.emblemUrl);
                if (tex != 0) {
                    ImGui::Image(tex, ImVec2(24.0f, 24.0f));
                    ImGui::SameLine();
                }
            }
            if (isSelected) {
                ImGui::PushStyleColor(ImGuiCol_Text, Design::ColPrimary);
                ImGui::TextUnformatted(c.name.c_str());
                ImGui::PopStyleColor();
            } else {
                ImGui::TextUnformatted(c.name.c_str());
            }
            ImGui::EndGroup();

            ImGui::SetCursorPosY(42.0f);
            Design::TextLabel(c.areaName.c_str());

            ImGui::SetCursorPosY(66.0f);
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
        if (service.isLoadingMatches()) {
            Design::EmptyState("Loading matches...");
        } else {
            Design::EmptyState("Matches have not been loaded yet.");
            if (Design::PrimaryButton("Load Matches")) service.loadMatches();
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
    
    // Show upcoming dates first (nearest first), then completed dates (newest first).
    std::time_t now = std::time(nullptr);
    std::tm utcNow{};
#ifdef _WIN32
    gmtime_s(&utcNow, &now);
#else
    gmtime_r(&now, &utcNow);
#endif
    std::ostringstream dateStream;
    dateStream << std::put_time(&utcNow, "%Y-%m-%d");
    const std::string today = dateStream.str();

    std::vector<std::string> orderedDates;
    auto firstUpcoming = byDate.lower_bound(today);
    for (auto it = firstUpcoming; it != byDate.end(); ++it) {
        orderedDates.push_back(it->first);
    }
    while (firstUpcoming != byDate.begin()) {
        --firstUpcoming;
        orderedDates.push_back(firstUpcoming->first);
    }

    for (const auto& date : orderedDates) {
        Design::SectionTitle(date.c_str());
        int rowIndex = 0;
        for (const auto& m : byDate.at(date)) {
            ImGui::PushID(m.id != 0 ? m.id : rowIndex);
            bool hasScore = (m.status == "FINISHED" || m.status == "IN_PLAY" || m.status == "PAUSED");
            int hScore = m.homeScoreFullTime.value_or(0);
            int aScore = m.awayScoreFullTime.value_or(0);
            Design::MatchRow(m.homeTeam.name, hScore, aScore, m.awayTeam.name, m.status, m.utcDate, hasScore, m.homeTeam.crestUrl, m.awayTeam.crestUrl);
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
        if (service.isLoadingStandings()) {
            Design::EmptyState("Loading standings...");
        } else {
            Design::EmptyState("Standings have not been loaded yet.");
            if (Design::PrimaryButton("Load Standings")) service.loadStandings();
        }
        return;
    }

    const auto& standings = service.getStandings();

    // Group rows by API group label (e.g. "Group A", "Group B"). Leagues use an empty key.
    std::map<std::string, std::vector<RealFootball::RealStanding>> byGroup;
    for (const auto& s : standings) {
        byGroup[s.groupName].push_back(s);
    }

    if (byGroup.size() > 1) {
        Design::TextMuted((std::to_string(byGroup.size()) + " groups — scroll down to see all").c_str());
        Design::Spacing();
    }

    auto renderStandingsTable = [](const char* tableId, const std::vector<RealFootball::RealStanding>& groupStandings) {
        // No ScrollY here: with ScrollY + full height, the first group table fills the panel
        // and hides every group below it.
        if (ImGui::BeginTable(tableId, 10, ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerH)) {
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

            ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextSecondary);
            ImGui::TableHeadersRow();
            ImGui::PopStyleColor();

            for (size_t i = 0; i < groupStandings.size(); ++i) {
                const auto& s = groupStandings[i];
                ImGui::PushID(static_cast<int>(i));
                ImGui::TableNextRow();

                ImGui::TableNextColumn();
                Design::TextLabel(std::to_string(s.position).c_str());

                ImGui::TableNextColumn();
                if (!s.team.crestUrl.empty()) {
                    ImTextureID tex = GetRealLogoCache().getTexture(s.team.crestUrl);
                    if (tex != 0) {
                        ImGui::Image(tex, ImVec2(18.0f, 18.0f));
                        ImGui::SameLine();
                    }
                }
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
    };

    ImGui::PushStyleColor(ImGuiCol_ChildBg, Design::ColSurface);
    ImGui::PushStyleColor(ImGuiCol_Border, Design::ColBorder);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);

    if (ImGui::BeginChild("standings_panel", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar)) {
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1,1,1,0.05f));
        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10, 8));

        int groupIndex = 0;
        for (const auto& groupEntry : byGroup) {
            ImGui::PushID(groupIndex);

            if (!groupEntry.first.empty()) {
                Design::SectionTitle(groupEntry.first.c_str());
                Design::Spacing();
            }

            std::string tableId = groupEntry.first.empty()
                ? ("standings_league_" + std::to_string(groupIndex))
                : ("standings_" + groupEntry.first);
            renderStandingsTable(tableId.c_str(), groupEntry.second);
            Design::Spacing(2);

            ImGui::PopID();
            ++groupIndex;
        }

        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor(2);
}
