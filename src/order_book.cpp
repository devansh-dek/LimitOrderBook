#include "order_book.hpp"
#include <iostream>

void OrderBook::add_order(const Order& order) {
    std::lock_guard<std::recursive_mutex> lock(book_mutex);
    if (order.side == Side::BUY) {
        buy_book[order.price].push(order);
    } else {
        sell_book[order.price].push(order);
    }
    order_index[order.order_id] = {order.price, order.side};
}

void OrderBook::cancel_order(int order_id) {
    std::lock_guard<std::recursive_mutex> lock(book_mutex);
    if (order_index.find(order_id) == order_index.end()) return;

    auto [price, side] = order_index[order_id];

    std::map<double, std::queue<Order>>* book;
    if (side == Side::BUY) {
        book = reinterpret_cast<std::map<double, std::queue<Order>>*>(&buy_book);
    } else {
        book = &sell_book;
    }

    auto& queue = (*book)[price];

    std::queue<Order> new_queue;
    while (!queue.empty()) {
        Order ord = queue.front(); queue.pop();
        if (ord.order_id != order_id) {
            new_queue.push(ord);
        }
    }
    queue = std::move(new_queue);

    if (queue.empty()) {
        book->erase(price);  // ✅ Fix this too
    }

    order_index.erase(order_id);
}


void OrderBook::modify_order(int order_id, double new_price, int new_qty) {
    std::lock_guard<std::recursive_mutex> lock(book_mutex);
    if (order_index.find(order_id) == order_index.end()) return;

    auto [old_price, side] = order_index[order_id];
    std::map<double, std::queue<Order>>* book;
    if (side == Side::BUY) {
        book = reinterpret_cast<std::map<double, std::queue<Order>>*>(&buy_book);
    } else {
        book = &sell_book;
    }
    auto& queue = (*book)[old_price];

    std::queue<Order> new_queue;

    Order modified_order(0, 0, side, OrderType::LIMIT, 0.0, 0); // use correct side
    bool found = false;

    while (!queue.empty()) {
        Order ord = queue.front(); queue.pop();
        if (ord.order_id == order_id) {
            ord.price = new_price;
            ord.quantity = new_qty;
            modified_order = ord;
            found = true;
        } else {
            new_queue.push(ord);
        }
    }

    queue = std::move(new_queue);

    if (queue.empty()) {
        book->erase(old_price);
    }

    order_index.erase(order_id);

    if (found) {
        add_order(modified_order);
    }
}


void OrderBook::print_top_levels(int depth) {
    std::lock_guard<std::recursive_mutex> lock(book_mutex);

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
