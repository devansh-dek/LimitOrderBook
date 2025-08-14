#include "gui.hpp"
#include "latency_metrics.hpp"
#include "thread_safe_queue.hpp"
#include <imgui.h>
#include <vector>
#include <algorithm>

extern LatencyMetrics queue_push_latency, queue_pop_latency, match_latency, gui_frame_latency;

void run_gui(OrderBook& book) {

    // Make the window take up the entire viewport
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("Limit Order Book Dashboard", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    // --- Order Entry Form ---
    static int order_type = 0; // 0=Limit, 1=Market, 2=Stop, 3=Stop-Limit
    static int side = 0;       // 0=Buy, 1=Sell
    static float price = 100.0f;
    static float stop_price = 100.0f;
    static int quantity = 1;
    static int next_order_id = 1000;
    static char* order_types[] = { (char*)"Limit", (char*)"Market", (char*)"Stop", (char*)"Stop-Limit" };
    static char* sides[] = { (char*)"Buy", (char*)"Sell" };

    ImGui::BeginChild("OrderEntry", ImVec2(0, 110), true);
    ImGui::Text("Order Entry");
    ImGui::Combo("Order Type", &order_type, order_types, 4);
    ImGui::Combo("Side", &side, sides, 2);
    if (order_type == 0 || order_type == 3) // Limit or Stop-Limit
        ImGui::InputFloat("Price", &price, 0.1f, 1.0f, "%.2f");
    if (order_type == 2 || order_type == 3) // Stop or Stop-Limit
        ImGui::InputFloat("Stop Price", &stop_price, 0.1f, 1.0f, "%.2f");
    ImGui::InputInt("Quantity", &quantity);
    if (ImGui::Button("Submit Order")) {
        // Create and submit the order
        Side s = side == 0 ? Side::BUY : Side::SELL;
        OrderType t = OrderType::LIMIT;
        if (order_type == 0) t = OrderType::LIMIT;
        if (order_type == 1) t = OrderType::MARKET;
        if (order_type == 2) t = OrderType::STOP;
        if (order_type == 3) t = OrderType::STOP_LIMIT;
        Order o = (t == OrderType::STOP || t == OrderType::STOP_LIMIT)
            ? Order(next_order_id++, ImGui::GetTime(), s, t, price, quantity, stop_price)
            : Order(next_order_id++, ImGui::GetTime(), s, t, price, quantity);
        extern ThreadSafeQueue<Order> order_queue;
        order_queue.push(o);
    }
    ImGui::EndChild();

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

    // --- Pending Stop/Stop-Limit Orders ---
    ImGui::Separator();
    ImGui::Text("Pending Stop/Stop-Limit Orders:");
    ImGui::BeginChild("PendingStops", ImVec2(0, 80), true);
    for (const auto& o : book.stop_orders) {
        ImGui::Text("ID: %d | %s %s | Qty: %d | Stop: %.2f | Limit: %.2f | Triggered: %s",
            o.order_id,
            o.side == Side::BUY ? "Buy" : "Sell",
            o.type == OrderType::STOP ? "Stop" : "Stop-Limit",
            o.quantity,
            o.stop_price,
            o.type == OrderType::STOP_LIMIT ? o.price : 0.0,
            o.triggered ? "Yes" : "No");
    }
    ImGui::EndChild();

    ImGui::End();
}
