#include "matcher.hpp"
#include <iostream>
#include <chrono>
#include <fstream>
#include <sys/stat.h>
using namespace std;

// Helper to check if file is empty
bool is_file_empty(const std::string& filename) {
    struct stat st;
    if (stat(filename.c_str(), &st) != 0) return true;
    return st.st_size == 0;
}

std::ofstream latency_log;

void Matcher::match_order(Order& incoming, OrderBook& book) {
    static bool header_written = false;
    if (!latency_log.is_open()) {
        latency_log.open("latency.csv", std::ios::app);
        if (!header_written && is_file_empty("latency.csv")) {
            latency_log << "incoming_id,matched_id,price,quantity,latency_ns\n";
            header_written = true;
        }
    }
    auto start = std::chrono::high_resolution_clock::now();
    std::map<double, std::queue<Order>>* opposite_book;
    if (incoming.side == Side::BUY) {
        opposite_book = &book.sell_book;
    } else {
        opposite_book = reinterpret_cast<std::map<double, std::queue<Order>>*>(&book.buy_book);
    }
    for (auto it = opposite_book->begin(); it != opposite_book->end() && incoming.quantity > 0; ) {
        double price_level = it->first;
        bool price_match = false;
        if (incoming.type == OrderType::MARKET) price_match = true;
        else if (incoming.side == Side::BUY) price_match = (incoming.price >= price_level);
        else price_match = (incoming.price <= price_level);
        if (!price_match) break;
        auto& queue = it->second;
        while (!queue.empty() && incoming.quantity > 0) {
            Order& top = queue.front();
            int trade_qty = std::min(incoming.quantity, top.quantity);
            std::cout << "Matched Order " << incoming.order_id
                      << " with Order " << top.order_id
                      << " at Price " << price_level
                      << " for Quantity " << trade_qty << std::endl;
            incoming.quantity -= trade_qty;
            top.quantity -= trade_qty;
            if (top.quantity == 0) {
                queue.pop();
            }
            if (incoming.quantity == 0) {
                incoming.status = OrderStatus::FILLED;
            } else {
                incoming.status = OrderStatus::PARTIALLY_FILLED;
            }
            // Write match info to CSV
            auto end = std::chrono::high_resolution_clock::now();
            auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
            latency_log << incoming.order_id << "," << top.order_id << "," << price_level << "," << trade_qty << "," << ns << "\n";
        }
        if (queue.empty()) {
            it = (*opposite_book).erase(it);
        } else {
            ++it;
        }
    }
    if (incoming.quantity > 0 && incoming.type == OrderType::LIMIT) {
        book.add_order(incoming);
    }
}
