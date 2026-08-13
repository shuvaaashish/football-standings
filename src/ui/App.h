#ifndef APP_UI_H
#define APP_UI_H

#include <string>

namespace UI {

enum class Page {
    Dashboard,
    RealCompetitions,
    RealMatches,
    RealStandings,
    CustomLeagues,
    CustomStandings,
    CustomFixtures,
    Admin
};

struct AppState {
    std::string username;
    std::string role;
    Page currentPage = Page::Dashboard;
};

class App {
public:
    App();
    ~App();

    void applyTheme();

    // Render the sidebar. Returns the width consumed by the sidebar.
    float renderSidebar(AppState& state, bool& logoutRequested);

private:
    void renderSidebarItem(const char* label, Page page, AppState& state);
    void renderSidebarSection(const char* title);
};

} // namespace UI

#endif
