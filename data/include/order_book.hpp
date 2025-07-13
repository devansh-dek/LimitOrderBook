#pragma once

#include <map>
#include <queue>
#include <mutex>
#include "order.hpp"
#include <unordered_map>

class OrderBook {
public:
    std::map<double, std::queue<Order>, std::greater<>> buy_book;
    std::map<double, std::queue<Order>> sell_book;
    std::unordered_map<int, std::pair<double, Side>> order_index;
    std::recursive_mutex book_mutex;

    void add_order(const Order& order);
    void cancel_order(int order_id);
    void modify_order(int order_id, double new_price, int new_qty);
    void print_top_levels(int depth = 5);
};
