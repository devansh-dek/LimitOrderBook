#pragma once

#include <map>
#include <queue>
#include <mutex>
#include "order.hpp"

class OrderBook {
public:
    // Buy side: high price to low (reverse order)
    std::map<double, std::queue<Order>, std::greater<>> buy_book;

    // Sell side: low price to high (normal order)
    std::map<double, std::queue<Order>> sell_book;

    std::mutex book_mutex;

    void add_order(const Order& order);
    void print_top_levels(int depth = 5);
};
