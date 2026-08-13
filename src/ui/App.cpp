#include "App.h"
#include "imgui.h"
#include "DesignSystem.h"

using namespace UI;

App::App() {
}

App::~App() {
}

void App::applyTheme() {
    Design::ApplyTheme();
    Design::LoadDefaultFont(ImGui::GetIO());
}

void App::renderSidebarSection(const char* title) {
    ImGui::Dummy(ImVec2(0, 12));
    ImGui::SetWindowFontScale(0.85f);
    ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextSecondary);
    ImGui::Text("%s", title);
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);
    ImGui::Dummy(ImVec2(0, 4));
}

void App::renderSidebarItem(const char* label, Page page, AppState& state) {
    bool active = (state.currentPage == page);

    if (active) {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.47f, 0.95f, 0.15f)); // Primary with alpha
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.14f, 0.47f, 0.95f, 0.25f));
        ImGui::PushStyleColor(ImGuiCol_Text, Design::ColPrimary);
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Design::ColSurface);
        ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextSecondary);
    }
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.0f, 0.5f)); // Left align

    if (ImGui::Button(label, ImVec2(-1, 36))) {
        state.currentPage = page;
    }
    
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor(3);
}

float App::renderSidebar(AppState& state, bool& logoutRequested) {
    const float sidebarWidth = 240.0f;
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;

    ImGui::PushStyleColor(ImGuiCol_WindowBg, Design::ColSurface); // Sidebar has surface color
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(20, 24));

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(sidebarWidth, displaySize.y));
    ImGui::Begin("##Sidebar", nullptr,
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBringToFrontOnFocus);

    // Brand
    ImGui::SetWindowFontScale(1.3f);
    ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextPrimary);
    ImGui::Text("Football Hub");
    ImGui::PopStyleColor();
    ImGui::SetWindowFontScale(1.0f);
    
    ImGui::Dummy(ImVec2(0, 8));
    
    // Draw subtle separator
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    drawList->AddLine(p, ImVec2(p.x + sidebarWidth - 40, p.y), ImGui::GetColorU32(Design::ColBorder));
    ImGui::Dummy(ImVec2(0, 4));

    // HOME
    renderSidebarSection("HOME");
    renderSidebarItem(" Dashboard", Page::Dashboard, state);

    // REAL FOOTBALL
    renderSidebarSection("REAL FOOTBALL");
    renderSidebarItem(" Competitions", Page::RealCompetitions, state);
    renderSidebarItem(" Matches", Page::RealMatches, state);
    renderSidebarItem(" Standings##RealFootball", Page::RealStandings, state);

    // MY LEAGUES
    renderSidebarSection("MY LEAGUES");
    renderSidebarItem(" My Leagues", Page::CustomLeagues, state);
    renderSidebarItem(" Fixtures", Page::CustomFixtures, state);
    renderSidebarItem(" Standings##MyLeagues", Page::CustomStandings, state);

    // ADMIN
    if (state.role == "Admin") {
        renderSidebarSection("ADMIN");
        renderSidebarItem(" Management", Page::Admin, state);
    }

    // User Profile Bottom Area
    float bottomHeight = 90.0f;
    float currentY = ImGui::GetCursorPosY();
    float availableY = ImGui::GetContentRegionMax().y;
    if (availableY - currentY > bottomHeight) {
        ImGui::SetCursorPosY(availableY - bottomHeight);
    }

    p = ImGui::GetCursorScreenPos();
    drawList->AddLine(p, ImVec2(p.x + sidebarWidth - 40, p.y), ImGui::GetColorU32(Design::ColBorder));
    ImGui::Dummy(ImVec2(0, 12));

    ImGui::PushStyleColor(ImGuiCol_Text, Design::ColTextPrimary);
    ImGui::Text("%s", state.username.c_str());
    ImGui::PopStyleColor();
    
    Design::TextLabel(state.role.c_str());
    ImGui::Dummy(ImVec2(0, 4));
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
    if (ImGui::Button("Logout", ImVec2(-1, 32))) {
        logoutRequested = true;
    }
    ImGui::PopStyleVar();

    ImGui::End();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();

    return sidebarWidth;
}
