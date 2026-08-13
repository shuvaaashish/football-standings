#include "LeaguesUI.h"
#include "DesignSystem.h"
#include "imgui.h"

using namespace UI;

void LeaguesUI::render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache, ViewerUiState& viewerUi) {
    Design::PageTitle("My Leagues");

    auto leagues = uiCache.getLeagues();
    if (leagues.empty()) {
        dbWorker.postTask([&uiCache](Database& db){ uiCache.refreshLeagues(db); });
        Design::EmptyState("No custom leagues available.");
    } else {
        float avail = ImGui::GetContentRegionAvail().x;
        int cols = std::max(1, static_cast<int>(avail / 280.0f));
        ImGui::Columns(cols, nullptr, false);

        for (size_t i = 0; i < leagues.size(); ++i) {
            const League& L = leagues[i];
            
            if (Design::BeginCard((std::string("league_card2_") + std::to_string(L.getId())).c_str(), ImVec2(0,120))) {
                ImGui::Text("%s", L.getName().c_str());
                Design::TextLabel((std::to_string(uiCache.getTeams(L.getId()).size()) + " Teams").c_str());
                
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetContentRegionAvail().y - 28.0f);
                
                if (Design::PrimaryButton(("Standings##" + std::to_string(L.getId())).c_str(), ImVec2(80, 0))) {
                    viewerUi.selectedLeagueIndex = static_cast<int>(i);
                    appState.currentPage = Page::CustomStandings;
                }
                ImGui::SameLine();
                if (ImGui::Button(("Fixtures##" + std::to_string(L.getId())).c_str(), ImVec2(80, 0))) {
                    viewerUi.selectedLeagueIndex = static_cast<int>(i);
                    appState.currentPage = Page::CustomFixtures;
                }
                
                Design::EndCard();
            }
            ImGui::NextColumn();
        }
        ImGui::Columns(1);
    }
}
