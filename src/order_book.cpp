#include "order_book.hpp"
#include <iostream>
#include <map>
#include <queue>
using namespace std;

void OrderBook::add_order(const Order& order) {
    lock_guard<recursive_mutex> lock(book_mutex);
    if (order.side == Side::BUY) {
        buy_book[order.price].push(order);
    } else {
        sell_book[order.price].push(order);
    }
    order_index[order.order_id] = {order.price, order.side};
}

void OrderBook::cancel_order(int order_id) {
    lock_guard<recursive_mutex> lock(book_mutex);
    if (order_index.find(order_id) == order_index.end()) return;

    auto [price, side] = order_index[order_id];

    std::map<double, std::queue<Order>>* book;
    if (side == Side::BUY) {
        book = reinterpret_cast<std::map<double, std::queue<Order>>*>(&buy_book);
    } else {
        book = &sell_book;
    }

    // Work on a copy, then assign back
    std::queue<Order> new_queue;
    {
        std::queue<Order>& orig_queue = (*book)[price];
        while (!orig_queue.empty()) {
            Order ord = orig_queue.front(); orig_queue.pop();
            if (ord.order_id != order_id) {
                new_queue.push(ord);
            }
        }
    }
    if (new_queue.empty()) {
        book->erase(price);
    } else {
        (*book)[price] = std::move(new_queue);
    }
    order_index.erase(order_id);
}

void OrderBook::modify_order(int order_id, double new_price, int new_qty) {
    lock_guard<recursive_mutex> lock(book_mutex);
    if (order_index.find(order_id) == order_index.end()) return;

    auto [old_price, side] = order_index[order_id];
    std::map<double, std::queue<Order>>* book;
    if (side == Side::BUY) {
        book = reinterpret_cast<std::map<double, std::queue<Order>>*>(&buy_book);
    } else {
        book = &sell_book;
    }

    std::queue<Order> new_queue;
    Order modified_order(0, 0, side, OrderType::LIMIT, 0.0, 0);
    bool found = false;
    {
        std::queue<Order>& orig_queue = (*book)[old_price];
        while (!orig_queue.empty()) {
            Order ord = orig_queue.front(); orig_queue.pop();
            if (ord.order_id == order_id) {
                ord.price = new_price;
                ord.quantity = new_qty;
                modified_order = ord;
                found = true;
            } else {
                new_queue.push(ord);
            }
        }
    }
    if (new_queue.empty()) {
        book->erase(old_price);
    } else {
        (*book)[old_price] = std::move(new_queue);
    }
    order_index.erase(order_id);
    if (found) {
        add_order(modified_order);
    }
}


void OrderBook::print_top_levels(int depth) {
    lock_guard<recursive_mutex> lock(book_mutex);

    cout << "=== ORDER BOOK ===" << endl;
    cout << "SELL SIDE:" << endl;

    int count = 0;
    for (const auto& [price, queue] : sell_book) {
        cout << "Price: " << price << " | Orders: " << queue.size() << endl;
        if (++count >= depth) break;
    }

    cout << "BUY SIDE:" << endl;
    count = 0;
    for (const auto& [price, queue] : buy_book) {
        cout << "Price: " << price << " | Orders: " << queue.size() << endl;
        if (++count >= depth) break;
    }

    cout << "==================" << endl;
}
