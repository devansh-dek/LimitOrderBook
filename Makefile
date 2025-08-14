LOB_BIN = lob
CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall

SRC = $(wildcard src/*.cpp)
INC = -I include

# Only use one copy of each ImGui source file (from src/), not from externals/imgui/
IMGUI_SRC = src/imgui.cpp src/imgui_draw.cpp src/imgui_tables.cpp src/imgui_widgets.cpp src/imgui_impl_glfw.cpp src/imgui_impl_opengl3.cpp src/imgui_demo.cpp

all:
	$(CXX) $(CXXFLAGS) src/gui.cpp src/main.cpp src/matcher.cpp src/order_book.cpp $(IMGUI_SRC) $(INC) -I./externals/imgui -I./src -o $(LOB_BIN) -lglfw -lGL -ldl -lpthread


# Build and run the basic order book test
test_order_book:
	$(CXX) $(CXXFLAGS) test/order_book_basic_test.cpp src/order_book.cpp $(INC) -o test/order_book_basic_test -lpthread
	./test/order_book_basic_test

clean:
	rm -f $(LOB_BIN) test/order_book_basic_test
