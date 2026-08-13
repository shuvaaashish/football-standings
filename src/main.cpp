#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "Database.h"
#include "ui/DbWorker.h"
#include "ui/UiCache.h"
#include "ui/App.h"
#include "ui/DashboardUI.h"
#include "ui/LeaguesUI.h"
#include "ui/StandingsUI.h"
#include "ui/FixturesUI.h"
#include "ui/AdminUI.h"
#include "ui/RealFootballUI.h"
#include "ui/UiState.h"
#include "api/FootballApiClient.h"
#include "api/RealFootballService.h"
#include <glfw3.h>
#include <iostream>
#include <cstdio>
#include <exception>
#include <string>
#include <vector>
#include <memory>

struct LoginState {
    bool loggedIn;
    bool showSignUp;
    User* currentUser;
    char username[128];
    char password[128];
    char signupUsername[128];
    char signupPassword[128];
    char signupConfirm[128];
    std::string errorMessage;
    std::string signupErrorMessage;
    std::string signupSuccessMessage;

    LoginState() : loggedIn(false), showSignUp(false), currentUser(nullptr) {
        username[0] = '\0';
        password[0] = '\0';
        signupUsername[0] = '\0';
        signupPassword[0] = '\0';
        signupConfirm[0] = '\0';
    }
};

static void resetLoginState(LoginState& loginState, UI::ViewerUiState& viewerUi, UI::AdminUiState& adminUi) {
    if (loginState.currentUser != nullptr) {
        delete loginState.currentUser;
        loginState.currentUser = nullptr;
    }
    loginState.loggedIn = false;
    loginState.showSignUp = false;
    loginState.username[0] = '\0';
    loginState.password[0] = '\0';
    loginState.signupUsername[0] = '\0';
    loginState.signupPassword[0] = '\0';
    loginState.signupConfirm[0] = '\0';
    loginState.errorMessage.clear();
    loginState.signupErrorMessage.clear();
    loginState.signupSuccessMessage.clear();
    viewerUi.errorMessage.clear();
    viewerUi.selectedLeagueIndex = -1;
    adminUi.errorMessage.clear();
    adminUi.successMessage.clear();
}

int main() {
    try {
        if (!glfwInit()) {
            fprintf(stderr, "Failed to initialize GLFW\n");
            return 1;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(1280, 720, "Football Hub", nullptr, nullptr);
        if (!window) {
            fprintf(stderr, "Failed to create GLFW window\n");
            glfwTerminate();
            return 1;
        }
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        (void)io;

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        // FootballApi::runApiConnectionTest(); // Disabled — using RealFootballService now

        DbWorker dbWorker;
        UiCache uiCache;
        FootballApi::RealFootballService realFootballService;

        UI::App app;
        app.applyTheme();

        LoginState loginState;
        UI::AppState appState;
        UI::ViewerUiState viewerUi;
        UI::AdminUiState adminUi;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            dbWorker.pollCompleted();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (!loginState.loggedIn || loginState.currentUser == nullptr) {
                // ════════════════════════════════════════════════════════
                // LOGIN SCREEN
                // ════════════════════════════════════════════════════════
                ImGui::SetNextWindowPos(ImVec2(0, 0));
                ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
                ImGui::Begin("Football Hub - Login", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

                if (!loginState.showSignUp) {
                    // Login form
                    ImGui::Text("Login");
                    ImGui::InputText("Username", loginState.username, sizeof(loginState.username));
                    ImGui::InputText("Password", loginState.password, sizeof(loginState.password), ImGuiInputTextFlags_Password);

                    if (!loginState.errorMessage.empty()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", loginState.errorMessage.c_str());
                    }

                    if (!loginState.signupSuccessMessage.empty()) {
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", loginState.signupSuccessMessage.c_str());
                    }

                    if (ImGui::Button("Login")) {
                        std::string uname(loginState.username);
                        std::string pword(loginState.password);
                        auto result = std::make_shared<User*>(nullptr);
                        auto errorMsg = std::make_shared<std::string>();

                        dbWorker.postTask(
                            [uname, pword, result, errorMsg](Database& db) {
                                try {
                                    *result = db.authenticateUser(uname, pword);
                                } catch (const DatabaseException& e) {
                                    *errorMsg = e.what();
                                }
                            },
                            [result, errorMsg, &loginState, &appState]() {
                                if (*result != nullptr) {
                                    loginState.currentUser = *result;
                                    loginState.loggedIn = true;
                                    loginState.errorMessage.clear();
                                    loginState.signupSuccessMessage.clear();
                                    appState.username = loginState.currentUser->getUsername();
                                    appState.role = loginState.currentUser->getRole();
                                    appState.currentPage = UI::Page::Dashboard;
                                } else {
                                    loginState.errorMessage = errorMsg->empty() ? "Login failed." : *errorMsg;
                                }
                            }
                        );
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Create Account")) {
                        loginState.showSignUp = true;
                        loginState.errorMessage.clear();
                        loginState.signupErrorMessage.clear();
                        loginState.signupSuccessMessage.clear();
                    }
                } else {
                    // Sign-up form
                    ImGui::Text("Create Account (Viewer)");
                    ImGui::InputText("Username", loginState.signupUsername, sizeof(loginState.signupUsername));
                    ImGui::InputText("Password", loginState.signupPassword, sizeof(loginState.signupPassword), ImGuiInputTextFlags_Password);
                    ImGui::InputText("Confirm Password", loginState.signupConfirm, sizeof(loginState.signupConfirm), ImGuiInputTextFlags_Password);

                    if (!loginState.signupErrorMessage.empty()) {
                        ImGui::TextColored(ImVec4(1.0f, 0.2f, 0.2f, 1.0f), "%s", loginState.signupErrorMessage.c_str());
                    }

                    if (!loginState.signupSuccessMessage.empty()) {
                        ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "%s", loginState.signupSuccessMessage.c_str());
                    }

                    if (ImGui::Button("Sign Up")) {
                        std::string pw(loginState.signupPassword);
                        std::string confirm(loginState.signupConfirm);

                        if (pw != confirm) {
                            loginState.signupErrorMessage = "Passwords do not match.";
                        } else {
                            std::string uname(loginState.signupUsername);
                            auto result = std::make_shared<std::string>();

                            dbWorker.postTask(
                                [uname, pw, result](Database& db) {
                                    try {
                                        db.registerViewer(uname, pw);
                                    } catch (const DatabaseException& e) {
                                        *result = e.what();
                                    }
                                },
                                [result, &loginState]() {
                                    if (result->empty()) {
                                        loginState.signupErrorMessage.clear();
                                        loginState.signupSuccessMessage = "Account created. Please log in.";

                                        std::snprintf(loginState.username, sizeof(loginState.username), "%s", loginState.signupUsername);
                                        loginState.password[0] = '\0';

                                        loginState.signupUsername[0] = '\0';
                                        loginState.signupPassword[0] = '\0';
                                        loginState.signupConfirm[0] = '\0';
                                        loginState.showSignUp = false;
                                    } else {
                                        loginState.signupErrorMessage = *result;
                                        loginState.signupSuccessMessage.clear();
                                    }
                                }
                            );
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::Button("Back to Login")) {
                        loginState.showSignUp = false;
                        loginState.signupErrorMessage.clear();
                        loginState.signupSuccessMessage.clear();
                    }
                }

                ImGui::End();
            } else {
                // ════════════════════════════════════════════════════════
                // MAIN APPLICATION — Sidebar + Content Area
                // Both Admin and Viewer share this layout.
                // ════════════════════════════════════════════════════════
                bool logoutRequested = false;

                // Render sidebar (returns width for content offset)
                float sidebarWidth = app.renderSidebar(appState, logoutRequested);

                if (logoutRequested) {
                    resetLoginState(loginState, viewerUi, adminUi);
                } else {
                    // Content area — positioned to the right of the sidebar
                    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
                    ImGui::SetNextWindowPos(ImVec2(sidebarWidth, 0));
                    ImGui::SetNextWindowSize(ImVec2(displaySize.x - sidebarWidth, displaySize.y));
                    ImGui::Begin("##Content", nullptr,
                        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
                        ImGuiWindowFlags_NoBringToFrontOnFocus);

                    // Route to the appropriate page
                    switch (appState.currentPage) {
                        case UI::Page::Dashboard:
                            UI::DashboardUI::render(appState, dbWorker, uiCache, realFootballService, viewerUi, loginState.currentUser);
                            break;

                        case UI::Page::RealCompetitions:
                            UI::RealFootballUI::renderCompetitions(realFootballService);
                            break;

                        case UI::Page::RealMatches:
                            UI::RealFootballUI::renderMatches(realFootballService);
                            break;

                        case UI::Page::RealStandings:
                            UI::RealFootballUI::renderStandings(realFootballService);
                            break;

                        case UI::Page::CustomLeagues:
                            UI::LeaguesUI::render(appState, dbWorker, uiCache, viewerUi);
                            break;

                        case UI::Page::CustomStandings:
                            UI::StandingsUI::render(appState, dbWorker, uiCache, viewerUi);
                            break;

                        case UI::Page::CustomFixtures:
                            UI::FixturesUI::render(appState, dbWorker, uiCache, viewerUi);
                            break;

                        case UI::Page::Admin:
                            if (appState.role == "Admin") {
                                UI::AdminUI::render(appState, dbWorker, uiCache, adminUi, loginState.currentUser);
                            } else {
                                ImGui::Text("Access denied. Admin privileges required.");
                            }
                            break;

                        default:
                            UI::DashboardUI::render(appState, dbWorker, uiCache, realFootballService, viewerUi, loginState.currentUser);
                            break;
                    }

                    ImGui::End(); // Content
                }
            }

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

        if (loginState.currentUser != nullptr) {
            delete loginState.currentUser;
        }

        // Ensure background worker shuts down cleanly before exiting.
        dbWorker.shutdown();

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    } catch (const DatabaseException& e) {
        fprintf(stderr, "Database error: %s\n", e.what());
    } catch (const std::exception& e) {
        fprintf(stderr, "Error: %s\n", e.what());
    }

    return 1;
}