CXX = g++
CXXFLAGS = -std=c++17 -Wall -Iinclude -Ilibs/imgui -Ilibs/imgui/backends
LDFLAGS = -lglfw -lGL -ldl -lpthread

IMGUI_SRC = libs/imgui/imgui.cpp \
            libs/imgui/imgui_draw.cpp \
            libs/imgui/imgui_tables.cpp \
            libs/imgui/imgui_widgets.cpp \
            libs/imgui/imgui_demo.cpp \
            libs/imgui/backends/imgui_impl_glfw.cpp \
            libs/imgui/backends/imgui_impl_opengl3.cpp

SRC = src/main.cpp $(IMGUI_SRC)

TARGET = football-standings

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean