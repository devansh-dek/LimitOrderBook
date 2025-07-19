#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include "order.hpp"
#include "order_book.hpp"
#include "matcher.hpp"
#include "thread_safe_queue.hpp"
#include "gui.hpp"
#include <GLFW/glfw3.h> // Include GLFW

// If you use glad for OpenGL loading, also add:
// #include <glad/glad.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "latency_metrics.hpp"

OrderBook book;
Matcher matcher;
ThreadSafeQueue<Order> order_queue;
std::mutex book_mutex; // Mutex to protect OrderBook

std::atomic<int> global_order_id = 1;
std::atomic<bool> matcher_done = false; // Flag to indicate matcher thread exit

LatencyMetrics queue_push_latency(500), queue_pop_latency(500), match_latency(500), gui_frame_latency(500);

// 🧠 Producer: randomly generates orders
void producer_func(int trader_id) {
    std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> qty_dist(1, 50);
    std::uniform_real_distribution<double> price_dist(99.0, 101.0);
    std::uniform_int_distribution<int> side_dist(0, 1);

    for (int i = 0; i < 100000; ++i) {
        Side side = (side_dist(rng) == 0) ? Side::BUY : Side::SELL;
        double price = price_dist(rng);
        int qty = qty_dist(rng);

        Order order(global_order_id++, std::chrono::system_clock::now().time_since_epoch().count(), side, OrderType::LIMIT, price, qty);
        auto t0 = std::chrono::high_resolution_clock::now();
        order_queue.push(order);
        auto t1 = std::chrono::high_resolution_clock::now();
        double micros = std::chrono::duration<double, std::micro>(t1 - t0).count();
        queue_push_latency.add(micros);
    }
}

// ⚙️ Consumer: matches orders
void matcher_func() {
    while (true) {
        auto t0 = std::chrono::high_resolution_clock::now();
        Order incoming = order_queue.pop();
        auto t1 = std::chrono::high_resolution_clock::now();
        double pop_micros = std::chrono::duration<double, std::micro>(t1 - t0).count();
        queue_pop_latency.add(pop_micros);
        if (incoming.order_id == -1) break; // Poison pill to exit
        {
            auto t2 = std::chrono::high_resolution_clock::now();
            std::lock_guard<std::mutex> lock(book_mutex);
            matcher.match_order(incoming, book);
            auto t3 = std::chrono::high_resolution_clock::now();
            double match_micros = std::chrono::duration<double, std::micro>(t3 - t2).count();
            match_latency.add(match_micros);
        }
    }
    matcher_done = true; // Signal done
}

int main() {
    // Initialize ImGui, create a window, and run the GUI loop
    // This is a minimal ImGui+GLFW+OpenGL3 setup for Linux
    // (You must have Dear ImGui, GLFW, and OpenGL3 installed and linked)
    
    // --- ImGui/GLFW/GL3 init ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(800, 600, "LOB Dashboard", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // 🧵 Spawn matcher
    std::thread matcher_thread(matcher_func);

    // 🧵 Spawn traders
    const int NUM_PRODUCERS = 4;
    std::vector<std::thread> producers;
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer_func, i + 1);
    }

    // --- Main loop ---
    bool matcher_crashed = false;
    while (!glfwWindowShouldClose(window)) {
        auto gui_t0 = std::chrono::high_resolution_clock::now();
        glfwPollEvents();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Lock book for GUI rendering
        {
            std::lock_guard<std::mutex> lock(book_mutex);
            run_gui(book); // Draw the order book dashboard
        }

        // Detect matcher thread exit (done or crash)
        if (!matcher_crashed && matcher_done.load()) {
            matcher_crashed = true;
            ImGui::OpenPopup("Matcher Error");
        }
        if (ImGui::BeginPopupModal("Matcher Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Matcher thread exited (completed, crashed, or received poison pill).\nThe GUI will remain open for inspection.");
            if (ImGui::Button("Close GUI")) {
                glfwSetWindowShouldClose(window, 1);
            }
            ImGui::EndPopup();
        }

        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
        auto gui_t1 = std::chrono::high_resolution_clock::now();
        double gui_micros = std::chrono::duration<double, std::micro>(gui_t1 - gui_t0).count();
        gui_frame_latency.add(gui_micros);
    }

    for (auto& t : producers) t.join();

    // Send poison pill to stop matcher (after all producers are done)
    order_queue.push(Order(-1, 0, Side::BUY, OrderType::LIMIT, 0.0, 0));
    matcher_thread.join();

    // --- Cleanup ---
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
