#include "gui.hpp"
#include "latency_metrics.hpp"
#include <imgui.h>
#include <vector>
#include <algorithm>

extern LatencyMetrics queue_push_latency, queue_pop_latency, match_latency, gui_frame_latency;

void run_gui(OrderBook& book) {
    // Make the window take up the entire viewport
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Limit Order Book Dashboard", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    ImGui::Text("Live Bid/Ask Depth");
    ImGui::Separator();

    // Optionally, use a child window for metrics for better contrast
    ImGui::BeginChild("MetricsChild", ImVec2(0, 250), true, ImGuiWindowFlags_NoMove);
    ImGui::Text("Performance Metrics (microseconds)");
    ImGui::Columns(2, nullptr, false);
    ImGui::Text("Queue Push"); ImGui::NextColumn();
    ImGui::Text("Avg: %.2f, Min: %.2f, Max: %.2f", queue_push_latency.avg(), queue_push_latency.min(), queue_push_latency.max()); ImGui::NextColumn();
    ImGui::Text("Queue Pop"); ImGui::NextColumn();
    ImGui::Text("Avg: %.2f, Min: %.2f, Max: %.2f", queue_pop_latency.avg(), queue_pop_latency.min(), queue_pop_latency.max()); ImGui::NextColumn();
    ImGui::Text("Order Match"); ImGui::NextColumn();
    ImGui::Text("Avg: %.2f, Min: %.2f, Max: %.2f", match_latency.avg(), match_latency.min(), match_latency.max()); ImGui::NextColumn();
    ImGui::Text("GUI Frame"); ImGui::NextColumn();
    ImGui::Text("Avg: %.2f, Min: %.2f, Max: %.2f", gui_frame_latency.avg(), gui_frame_latency.min(), gui_frame_latency.max()); ImGui::NextColumn();
    ImGui::Columns(1);
    ImGui::Spacing();
    // Plot latency history
    auto plot = [](const char* label, LatencyMetrics& m) {
        auto samples = m.get_samples();
        if (!samples.empty()) {
            std::vector<float> float_samples(samples.begin(), samples.end());
            float max_val = *std::max_element(float_samples.begin(), float_samples.end());
            ImGui::PlotLines(label, float_samples.data(), float_samples.size(), 0, nullptr, 0.0f, max_val, ImVec2(0, 60));
        }
    };
    plot("Queue Push Latency", queue_push_latency);
    plot("Queue Pop Latency", queue_pop_latency);
    plot("Order Match Latency", match_latency);
    plot("GUI Frame Latency", gui_frame_latency);
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::EndChild(); // Correctly close the metrics child window

    // Use child windows for better layout
    float mid = ImGui::GetContentRegionAvail().x * 0.5f;
    ImGui::Columns(2, nullptr, false);

    // --- Bids (left) ---
    ImGui::Text("Bids");
    ImGui::Separator();
    ImGui::BeginChild("BidsChild", ImVec2(mid - 10, 0), true);
    ImGui::Columns(2, nullptr, false);
    ImGui::Text("Price"); ImGui::NextColumn();
    ImGui::Text("Size"); ImGui::NextColumn();
    std::vector<std::pair<double, int>> bids;
    for (const auto& [price, queue] : book.buy_book) {
        int qty = 0;
        std::queue<Order> q = queue;
        while (!q.empty()) { qty += q.front().quantity; q.pop(); }
        bids.emplace_back(price, qty);
    }
    std::sort(bids.begin(), bids.end(), std::greater<>());
    for (const auto& [price, qty] : bids) {
        ImGui::Text("%.2f", price); ImGui::NextColumn();
        ImGui::Text("%d", qty); ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::NextColumn();

    // --- Asks (right) ---
    ImGui::Text("Asks");
    ImGui::Separator();
    ImGui::BeginChild("AsksChild", ImVec2(0, 0), true);
    ImGui::Columns(2, nullptr, false);
    ImGui::Text("Size"); ImGui::NextColumn();
    ImGui::Text("Price"); ImGui::NextColumn();
    std::vector<std::pair<double, int>> asks;
    for (const auto& [price, queue] : book.sell_book) {
        int qty = 0;
        std::queue<Order> q = queue;
        while (!q.empty()) { qty += q.front().quantity; q.pop(); }
        asks.emplace_back(price, qty);
    }
    std::sort(asks.begin(), asks.end());
    for (const auto& [price, qty] : asks) {
        ImGui::Text("%d", qty); ImGui::NextColumn();
        ImGui::Text("%.2f", price); ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::EndChild();
    ImGui::Columns(1);

    ImGui::End();
}
