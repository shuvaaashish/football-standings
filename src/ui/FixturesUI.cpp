#include "FixturesUI.h"
#include "DesignSystem.h"
#include "imgui.h"
#include <map>
#include <vector>

using namespace UI;

void FixturesUI::render(AppState& appState, DbWorker& dbWorker, UiCache& uiCache, ViewerUiState& viewerUi) {
    Design::PageTitle("My League Fixtures & Results");

    auto leagues = uiCache.getLeagues();
    if (viewerUi.selectedLeagueIndex < 0 || viewerUi.selectedLeagueIndex >= static_cast<int>(leagues.size())) {
        Design::EmptyState("Select a league first.");
        if (!leagues.empty()) {
            if (Design::PrimaryButton("Go to My Leagues")) appState.currentPage = Page::CustomLeagues;
        }
        return;
    }

    int lid = leagues[viewerUi.selectedLeagueIndex].getId();
    Design::TextMuted(("League: " + leagues[viewerUi.selectedLeagueIndex].getName()).c_str());
    Design::Spacing(2);

    auto matches = uiCache.getMatches(lid);
    if (matches.empty()) {
        dbWorker.postTask([lid, &uiCache](Database& db){ uiCache.refreshMatches(db, lid); });
        Design::EmptyState("Loading matches or no matches found.");
        return;
    }

    // Group by date
    std::map<std::string, std::vector<Match>> byDate;
    for (const auto& m : matches) byDate[m.getMatchDate()].push_back(m);

    for (auto it = byDate.rbegin(); it != byDate.rend(); ++it) {
        Design::SectionTitle(it->first.c_str());
        size_t rowIndex = 0;
        for (const auto& m : it->second) {
            ImGui::PushID(static_cast<int>(rowIndex));
            Design::MatchRow(m.getHomeTeamName(), m.getHomeScore(), m.getAwayScore(), m.getAwayTeamName(), "FINISHED", it->first, true);
            ImGui::PopID();
            ++rowIndex;
        }
        Design::Spacing(2);
    }
}
