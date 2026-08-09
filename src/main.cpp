#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include "Database.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <exception>

static void glfw_error_callback(int error, const char* description) {
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

int main() {
    try {
        Database database;

        printf("Database initialized successfully. Checking tables...\n");
        const char* tables[] = {"leagues", "teams", "players", "matches", "users"};
        bool allTablesExist = true;

        FILE* proofFile = fopen("data/startup-check.txt", "w");
        if (proofFile != nullptr) {
            fprintf(proofFile, "Database initialized successfully.\n");
        }

        for (int i = 0; i < 5; ++i) {
            bool exists = database.tableExists(tables[i]);
            printf("  %s: %s\n", tables[i], exists ? "created" : "missing");
            if (!exists) {
                allTablesExist = false;
            }

            if (proofFile != nullptr) {
                fprintf(proofFile, "%s: %s\n", tables[i], exists ? "created" : "missing");
            }
        }

        if (allTablesExist) {
            printf("All required tables are present.\n");
            if (proofFile != nullptr) {
                fprintf(proofFile, "All required tables are present.\n");
            }
        } else {
            printf("One or more required tables are missing.\n");
            if (proofFile != nullptr) {
                fprintf(proofFile, "One or more required tables are missing.\n");
            }
        }

        if (proofFile != nullptr) {
            fclose(proofFile);
        }

        glfwSetErrorCallback(glfw_error_callback);
        if (!glfwInit())
            return 1;

        const char* glsl_version = "#version 130";
        GLFWwindow* window = glfwCreateWindow(1280, 720, "Football Standings - Test Window", nullptr, nullptr);
        if (window == nullptr)
            return 1;

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1); // vsync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init(glsl_version);

        bool show_demo_window = true;

        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (show_demo_window)
                ImGui::ShowDemoWindow(&show_demo_window);

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);
        }

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