CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -Isrc -Ilibs/imgui -Ilibs/imgui/backends
LDFLAGS = -lglfw3 -lopengl32 -lsqlite3 -lcurl

IMGUI_SRC = libs/imgui/imgui.cpp \
            libs/imgui/imgui_draw.cpp \
            libs/imgui/imgui_tables.cpp \
            libs/imgui/imgui_widgets.cpp \
            libs/imgui/imgui_demo.cpp \
            libs/imgui/backends/imgui_impl_glfw.cpp \
            libs/imgui/backends/imgui_impl_opengl3.cpp

SRC = src/main.cpp $(IMGUI_SRC)
SRC += src/Database.cpp src/User.cpp src/Team.cpp src/League.cpp src/Match.cpp
SRC += src/ui/DbWorker.cpp src/ui/UiCache.cpp src/ui/App.cpp src/ui/DashboardUI.cpp src/ui/LeaguesUI.cpp src/ui/StandingsUI.cpp src/ui/FixturesUI.cpp src/ui/RealFootballUI.cpp src/ui/DesignSystem.cpp
SRC += src/ui/AdminUI.cpp src/api/FootballApiClient.cpp src/api/RealFootballService.cpp

TARGET = football-standings

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean