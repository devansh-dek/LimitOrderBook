# Dear ImGui + GLFW + OpenGL3 Integration for Limit Order Book Dashboard

## 1. Install Dependencies

```sh
sudo apt-get update
sudo apt-get install libglfw3-dev libgl1-mesa-dev libglu1-mesa-dev
```

## 2. Download Dear ImGui

```sh
cd /home/devanshkhandelwal/resumeProj/LimitBookOrder
mkdir -p externals
cd externals
git clone https://github.com/ocornut/imgui.git
```

## 3. Copy ImGui Backends

```sh
cd imgui
cp backends/imgui_impl_glfw.* ../../src/
cp backends/imgui_impl_opengl3.* ../../src/
cp *.cpp *.h ../../src/
```

## 4. Update Your Makefile

Add these to your Makefile's sources and includes:

- **Sources:**
  - src/imgui_impl_glfw.cpp
  - src/imgui_impl_opengl3.cpp
  - src/imgui.cpp
  - src/imgui_draw.cpp
  - src/imgui_tables.cpp
  - src/imgui_widgets.cpp
  - src/imgui_demo.cpp (optional)
- **Includes:**
  - -I./src -I./externals/imgui -I./src
- **Libraries:**
  - -lglfw -lGL -ldl -lpthread

Example Makefile snippet:

```
LOB_OBJS = src/main.o src/gui.o src/order_book.o src/matcher.o \
           src/imgui.o src/imgui_draw.o src/imgui_tables.o src/imgui_widgets.o \
           src/imgui_impl_glfw.o src/imgui_impl_opengl3.o

LOB_LIBS = -lglfw -lGL -ldl -lpthread

LOB_CXXFLAGS = -I./src -I./externals/imgui -I./src

lob: $(LOB_OBJS)
	$(CXX) -o $@ $^ $(LOB_LIBS)
```

## 5. Build and Run

```sh
make
./lob
```

---

**You now have a real-time ImGui dashboard for your Limit Order Book!**

- To customize the GUI, edit `src/gui.cpp`.
- To add more analytics, add more ImGui widgets.
