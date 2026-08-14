#include "DesignSystem.h"
#include "ImageCache.h"

#include <cstdio>
#include <fstream>
#include <string>

namespace {

UI::ImageCache& GetImageCache() {
    static UI::ImageCache cache;
    return cache;
}

bool TryLoadFontFromPath(const char* path, float size) {
    std::ifstream file(path);
    if (!file.good()) {
        return false;
    }
    ImFont* font = ImGui::GetIO().Fonts->AddFontFromFileTTF(path, size);
    return font != nullptr;
}

} // namespace

namespace UI {
namespace Design {

    const ImVec4 ColBackground    = ImVec4(0.05f, 0.06f, 0.07f, 1.00f); // #0D0F12
    const ImVec4 ColSurface       = ImVec4(0.11f, 0.13f, 0.15f, 1.00f); // #1A1F24
    const ImVec4 ColSurfaceAlt    = ImVec4(0.15f, 0.17f, 0.20f, 1.00f); // #212933
    const ImVec4 ColSurfaceHover  = ImVec4(0.17f, 0.20f, 0.24f, 1.00f); // #2A313A
    const ImVec4 ColPrimary       = ImVec4(0.17f, 0.49f, 0.98f, 1.00f); // electric blue
    const ImVec4 ColPrimaryHover  = ImVec4(0.26f, 0.60f, 1.00f, 1.00f);
    const ImVec4 ColTextPrimary   = ImVec4(0.96f, 0.97f, 0.98f, 1.00f);
    const ImVec4 ColTextSecondary = ImVec4(0.69f, 0.72f, 0.77f, 1.00f);
    const ImVec4 ColBorder        = ImVec4(0.24f, 0.27f, 0.32f, 1.00f);
    const ImVec4 ColSuccess       = ImVec4(0.19f, 0.81f, 0.55f, 1.00f);
    const ImVec4 ColWarning       = ImVec4(0.98f, 0.67f, 0.18f, 1.00f);
    const ImVec4 ColError         = ImVec4(0.90f, 0.30f, 0.26f, 1.00f);

    void ApplyTheme() {
        ImGuiStyle& style = ImGui::GetStyle();

        style.Colors[ImGuiCol_WindowBg] = ColBackground;
        style.Colors[ImGuiCol_ChildBg] = ColBackground;
        style.Colors[ImGuiCol_PopupBg] = ColSurface;
        style.Colors[ImGuiCol_FrameBg] = ColSurface;
        style.Colors[ImGuiCol_FrameBgHovered] = ColSurfaceHover;
        style.Colors[ImGuiCol_FrameBgActive] = ColSurfaceHover;
        style.Colors[ImGuiCol_TitleBg] = ColBackground;
        style.Colors[ImGuiCol_TitleBgActive] = ColBackground;
        style.Colors[ImGuiCol_Header] = ColSurface;
        style.Colors[ImGuiCol_HeaderHovered] = ColSurfaceHover;
        style.Colors[ImGuiCol_HeaderActive] = ColPrimary;
        style.Colors[ImGuiCol_Button] = ColSurface;
        style.Colors[ImGuiCol_ButtonHovered] = ColSurfaceHover;
        style.Colors[ImGuiCol_ButtonActive] = ColPrimary;
        style.Colors[ImGuiCol_Text] = ColTextPrimary;
        style.Colors[ImGuiCol_TextDisabled] = ColTextSecondary;
        style.Colors[ImGuiCol_Border] = ColBorder;
        style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_Tab] = ColSurface;
        style.Colors[ImGuiCol_TabHovered] = ColSurfaceHover;
        style.Colors[ImGuiCol_TabActive] = ColPrimary;
        style.Colors[ImGuiCol_TableHeaderBg] = ColSurfaceAlt;
        style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
        style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.0f, 1.0f, 1.0f, 0.02f);
        style.Colors[ImGuiCol_CheckMark] = ColPrimary;
        style.Colors[ImGuiCol_SliderGrab] = ColPrimary;
        style.Colors[ImGuiCol_SliderGrabActive] = ColPrimaryHover;

        style.WindowRounding = 10.0f;
        style.ChildRounding = 12.0f;
        style.FrameRounding = 8.0f;
        style.PopupRounding = 10.0f;
        style.ScrollbarRounding = 10.0f;
        style.GrabRounding = 8.0f;
        style.TabRounding = 8.0f;
        style.WindowBorderSize = 0.0f;
        style.ChildBorderSize = 1.0f;
        style.FrameBorderSize = 1.0f;
        style.WindowPadding = ImVec2(18.0f, 18.0f);
        style.FramePadding = ImVec2(12.0f, 9.0f);
        style.ItemSpacing = ImVec2(10.0f, 12.0f);
        style.ItemInnerSpacing = ImVec2(10.0f, 8.0f);
        style.CellPadding = ImVec2(10.0f, 8.0f);
        style.SeparatorTextPadding = ImVec2(0.0f, 8.0f);
    }

    void LoadDefaultFont(ImGuiIO& io) {
        if (io.Fonts->Fonts.empty()) {
            const char* candidates[] = {
                "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
                "/usr/share/fonts/truetype/liberation2/LiberationSans-Regular.ttf",
                "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
                "C:/Windows/Fonts/SegoeUI.ttf",
                "C:/Windows/Fonts/arial.ttf",
                ""};

            for (const char* path : candidates) {
                if (path[0] == '\0') {
                    break;
                }
                if (TryLoadFontFromPath(path, 16.0f)) {
                    return;
                }
            }
        }
    }

    void PageTitle(const char* title) {
        ImGui::SetWindowFontScale(1.45f);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextPrimary);
        ImGui::TextUnformatted(title);
        ImGui::PopStyleColor();
        ImGui::SetWindowFontScale(1.0f);
        Spacing(2);
    }

    void SectionTitle(const char* title) {
        ImGui::SetWindowFontScale(1.12f);
        ImGui::PushStyleColor(ImGuiCol_Text, ColTextPrimary);
        ImGui::TextUnformatted(title);
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
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ColSurfaceAlt);
        ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.0f, 14.0f));
        // BeginChild() must always be paired with EndChild(), even when it
        // returns false because the card is clipped. All card call sites use
        // the return value to decide whether to call EndCard(), so keep the
        // body active and guarantee that pairing.
        ImGui::BeginChild(id, size, true, 0);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(2);
        return true;
    }

    void EndCard() {
        ImGui::EndChild();
    }

    void Badge(const char* text, const ImVec4& color) {
        ImVec4 bgColor = color;
        bgColor.w = 0.15f;

        ImGui::PushStyleColor(ImGuiCol_Button, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, bgColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, bgColor);
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
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

    void MatchRow(const std::string& homeTeam, int homeScore, int awayScore, const std::string& awayTeam, const std::string& status, const std::string& date, bool hasScore,
                  const std::string& homeCrestUrl, const std::string& awayCrestUrl) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ColSurfaceAlt);
        ImGui::PushStyleColor(ImGuiCol_Border, ColBorder);
        ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.0f);

        std::string id = "mr_" + homeTeam + awayTeam + date;
        ImGui::BeginChild(id.c_str(), ImVec2(0, 52), true);

        float availWidth = ImGui::GetContentRegionAvail().x;
        float centerW = 170.0f;

        ImGui::SetCursorPosY(14.0f);
        ImGui::BeginGroup();
        if (!homeCrestUrl.empty()) {
            ImTextureID tex = GetImageCache().getTexture(homeCrestUrl);
            if (tex != 0) {
                ImGui::Image(tex, ImVec2(18.0f, 18.0f));
                ImGui::SameLine();
            }
        }
        ImGui::TextUnformatted(homeTeam.c_str());
        ImGui::EndGroup();

        ImGui::SetCursorPosX((availWidth - centerW) * 0.5f);
        ImGui::SetCursorPosY(12.0f);
        ImGui::BeginGroup();
        if (hasScore) {
            ImGui::Text("%d  -  %d", homeScore, awayScore);
        } else {
            ImGui::Text(" vs ");
        }
        ImGui::EndGroup();

        ImGui::SetCursorPosX((availWidth - centerW) * 0.5f + 76.0f);
        ImGui::SetCursorPosY(9.0f);
        StatusBadge(status);

        float rightAlign = availWidth - ImGui::CalcTextSize(awayTeam.c_str()).x - 10.0f;
        ImGui::SetCursorPosX(rightAlign);
        ImGui::SetCursorPosY(14.0f);
        ImGui::BeginGroup();
        if (!awayCrestUrl.empty()) {
            ImGui::SameLine();
            ImTextureID tex = GetImageCache().getTexture(awayCrestUrl);
            if (tex != 0) {
                ImGui::Image(tex, ImVec2(18.0f, 18.0f));
                ImGui::SameLine();
            }
        }
        ImGui::TextUnformatted(awayTeam.c_str());
        ImGui::EndGroup();

        ImGui::EndChild();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
    }

    bool PrimaryButton(const char* label, const ImVec2& size) {
        ImGui::PushStyleColor(ImGuiCol_Button, ColPrimary);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ColPrimaryHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ColPrimary);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,1));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
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
