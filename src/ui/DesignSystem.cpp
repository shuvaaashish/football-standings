#include "DesignSystem.h"

namespace UI {
namespace Design {

    // Modern dark theme colors
    const ImVec4 ColBackground    = ImVec4(0.06f, 0.07f, 0.09f, 1.00f); // #0F1117
    const ImVec4 ColSurface       = ImVec4(0.12f, 0.13f, 0.16f, 1.00f); // #1E2129
    const ImVec4 ColSurfaceHover  = ImVec4(0.16f, 0.18f, 0.22f, 1.00f); // #292E38
    const ImVec4 ColPrimary       = ImVec4(0.14f, 0.47f, 0.95f, 1.00f); // #2478F2
    const ImVec4 ColPrimaryHover  = ImVec4(0.20f, 0.55f, 1.00f, 1.00f); // #338CFF
    const ImVec4 ColTextPrimary   = ImVec4(0.96f, 0.96f, 0.96f, 1.00f); // #F5F5F5
    const ImVec4 ColTextSecondary = ImVec4(0.60f, 0.62f, 0.68f, 1.00f); // #999EAD
    const ImVec4 ColBorder        = ImVec4(0.20f, 0.21f, 0.25f, 1.00f); // #333640
    const ImVec4 ColSuccess       = ImVec4(0.18f, 0.79f, 0.49f, 1.00f); // #2ECA7D
    const ImVec4 ColWarning       = ImVec4(0.95f, 0.61f, 0.07f, 1.00f); // #F29C12
    const ImVec4 ColError         = ImVec4(0.90f, 0.30f, 0.26f, 1.00f); // #E64C42

    void PageTitle(const char* title) {
        ImGui::SetWindowFontScale(1.4f);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextPrimary);
        ImGui::Text("%s", title);
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        Spacing(2);
    }

    void SectionTitle(const char* title) {
        ImGui::SetWindowFontScale(1.15f);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextPrimary);
        ImGui::Text("%s", title);
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        Spacing(1);
    }

    void TextMuted(const char* text) {
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        ImGui::TextUnformatted(text);
        ImGui::PopStyleColor();
    }

    void TextLabel(const char* label) {
        ImGui::SetWindowFontScale(0.9f);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        ImGui::TextUnformatted(label);
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
    }

    void Spacing(int count) {
        for (int i = 0; i < count; ++i) {
            ImGui::Spacing();
        }
    }

    bool BeginCard(const char* id, const ImVec2& size) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ColSurface);
        ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16.0f, 16.0f));
        
        bool ret = ImGui::BeginChild(id, size, true, 0);
        
        // Pop styling so it doesn't affect inner elements
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        return ret;
    }

    void EndCard() {
        ImGui::EndChild();
    }

    void Badge(const char* text, const ImVec4& color) {
        ImVec4 bgColor = color;
        bgColor.w = 0.2f; // Make background transparent
        
        ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgColor);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
        
        // Use a dummy button to draw the badge
        ImGui::Button(text);
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
    }

    void StatusBadge(const std::string& status) {
        if (status == "FINISHED" || status == "COMPLETED") {
            Badge("FT", ColSuccess);
        } else if (status == "IN_PLAY" || status == "LIVE") {
            Badge("LIVE", ColWarning);
        } else if (status == "PAUSED") {
            Badge("HT", ColWarning);
        } else if (status == "SCHEDULED" || status == "TIMED") {
            Badge("UPCOMING", ColTextSecondary);
        } else {
            Badge(status.c_str(), ColTextSecondary);
        }
    }

    void MatchRow(const std::string& homeTeam, int homeScore, int awayScore, const std::string& awayTeam, const std::string& status, const std::string& date, bool hasScore) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ColSurface);
        ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 6.0f);
        
        // Generate a unique ID using the teams and date
        std::string id = "mr_" + homeTeam + awayTeam + date;
        
        ImGui::BeginChild(id.c_str(), ImVec2(0, 48), true);
        
        float availWidth = ImGui::GetContentRegionAvail().x;
        
        // Left: Home Team
        ImGui::SetCursorPos(ImVec2(16, 15));
        ImGui::TextUnformatted(homeTeam.c_str());
        
        // Center: Score & Status
        float centerW = 200.0f;
        ImGui::SetCursorPos(ImVec2((availWidth - centerW) * 0.5f, 15));
        
        ImGui::BeginGroup();
        if (hasScore) {
            ImGui::Text("%d  -  %d", homeScore, awayScore);
        } else {
            ImGui::Text(" vs ");
        }
        ImGui::EndGroup();
        
        ImGui::SameLine();
        ImGui::SetCursorPosX((availWidth - centerW) * 0.5f + 80.0f);
        ImGui::SetCursorPosY(11);
        StatusBadge(status);
        
        // Right: Away Team & Date
        float rightAlign = availWidth - ImGui::CalcTextSize(awayTeam.c_str()).x - 16.0f;
        ImGui::SetCursorPos(ImVec2(rightAlign, 15));
        ImGui::TextUnformatted(awayTeam.c_str());
        
        // Date below status if available
        if (!date.empty()) {
            std::string dateStr = date.substr(0, 10);
            ImGui::SetCursorPosX((availWidth - centerW) * 0.5f + 140.0f);
            ImGui::SetCursorPosY(16);
            TextLabel(dateStr.c_str());
        }

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }

    bool PrimaryButton(const char* label, const ImVec2& size) {
        ImGui::PushStyleColor(ImGuiCol_Button, ColPrimary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColPrimaryHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColPrimary); // Keeping it simple
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
        
        bool pressed = ImGui::Button(label, size);
        
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(4);
        return pressed;
    }

    void EmptyState(const char* message) {
        ImGui::Spacing();
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextSecondary);
        
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize(message).x;
        
        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::TextUnformatted(message);
        
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Spacing();
    }

} // namespace Design
} // namespace UI
