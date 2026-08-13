#ifndef DESIGN_SYSTEM_H
#define DESIGN_SYSTEM_H

#include "imgui.h"
#include <string>

namespace UI {
namespace Design {

    // ─── Colors ─────────────────────────────────────────────────────────────
    extern const ImVec4 ColBackground;
    extern const ImVec4 ColSurface;
    extern const ImVec4 ColSurfaceAlt;
    extern const ImVec4 ColSurfaceHover;
    extern const ImVec4 ColPrimary;
    extern const ImVec4 ColPrimaryHover;
    extern const ImVec4 ColTextPrimary;
    extern const ImVec4 ColTextSecondary;
    extern const ImVec4 ColBorder;
    extern const ImVec4 ColSuccess;
    extern const ImVec4 ColWarning;
    extern const ImVec4 ColError;

    void ApplyTheme();
    void LoadDefaultFont(ImGuiIO& io);

    // ─── Typography Helpers ─────────────────────────────────────────────────
    void PageTitle(const char* title);
    void SectionTitle(const char* title);
    void TextMuted(const char* text);
    void TextLabel(const char* label);

    // ─── Layout & Components ────────────────────────────────────────────────
    void Spacing(int count = 1);
    
    // Cards
    bool BeginCard(const char* id, const ImVec2& size = ImVec2(0, 0));
    void EndCard();

    // Badges
    void Badge(const char* text, const ImVec4& color);
    
    // Status badges (standardized)
    void StatusBadge(const std::string& status);

    // Matches
    void MatchRow(const std::string& homeTeam, int homeScore, int awayScore, const std::string& awayTeam, const std::string& status, const std::string& date, bool hasScore,
                  const std::string& homeCrestUrl = "", const std::string& awayCrestUrl = "");

    // Buttons
    bool PrimaryButton(const char* label, const ImVec2& size = ImVec2(0, 0));

    // Empty States
    void EmptyState(const char* message);

} // namespace Design
} // namespace UI

#endif
