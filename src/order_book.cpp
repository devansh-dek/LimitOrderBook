#include "order_book.hpp"
#include <iostream>

void OrderBook::add_order(const Order& order) {
    std::lock_guard<std::mutex> lock(book_mutex);

    if (order.side == Side::BUY) {
        buy_book[order.price].push(order);
    } else {
        sell_book[order.price].push(order);
    }
}

void OrderBook::print_top_levels(int depth) {
    std::lock_guard<std::mutex> lock(book_mutex);

    std::cout << "=== ORDER BOOK ===" << std::endl;
    std::cout << "SELL SIDE:" << std::endl;

    int count = 0;
    for (const auto& [price, queue] : sell_book) {
        std::cout << "Price: " << price << " | Orders: " << queue.size() << std::endl;
        if (++count >= depth) break;
    }

    std::cout << "BUY SIDE:" << std::endl;
    count = 0;
    for (const auto& [price, queue] : buy_book) {
        std::cout << "Price: " << price << " | Orders: " << queue.size() << std::endl;
        if (++count >= depth) break;
    }

    std::cout << "==================" << std::endl;
}
