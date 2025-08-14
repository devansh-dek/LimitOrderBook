#include "order_book.hpp"
#include <iostream>
#include <map>
#include <queue>
#include <algorithm> // For std::remove_if
using namespace std;

// Add an order to the book. Handles LIMIT, MARKET, STOP, and STOP_LIMIT orders.
// MARKET orders match immediately with the best available price on the opposite side.
// STOP and STOP_LIMIT orders are stored until triggered by price movement.
void OrderBook::add_order(const Order& order) {
    lock_guard<recursive_mutex> lock(book_mutex);

    // Handle STOP and STOP_LIMIT orders: store until triggered
    if (order.type == OrderType::STOP || order.type == OrderType::STOP_LIMIT) {
        // For beginners: stop orders are not active until the market price crosses the stop price.
        stop_orders.push_back(order);
        std::cout << "Stop order stored (OrderID: " << order.order_id << ", Stop Price: " << order.stop_price << ")\n";
        return;
    }

    if (order.type == OrderType::MARKET) {
        // MARKET ORDER LOGIC
        int remaining = order.quantity;
        if (order.side == Side::BUY) {
            // Buy market order: match with lowest sell prices
            while (remaining > 0 && !sell_book.empty()) {
                auto it = sell_book.begin(); // lowest price
                std::queue<Order>& queue = it->second;
                while (remaining > 0 && !queue.empty()) {
                    Order top = queue.front(); queue.pop();
                    int fill_qty = std::min(remaining, top.quantity - top.filled);
                    // Fill the order
                    remaining -= fill_qty;
                    top.filled += fill_qty;
                    // Print fill info (for demo)
                    std::cout << "Market BUY filled " << fill_qty << " @ " << it->first << " (OrderID: " << top.order_id << ")\n";
                    if (top.filled < top.quantity) {
                        queue.push(top); // Put back partially filled order
                    } else {
                        // Fully filled, remove from index
                        order_index.erase(top.order_id);
                    }
                }
                if (queue.empty()) sell_book.erase(it);
            }
        } else {
            // Sell market order: match with highest buy prices
            while (remaining > 0 && !buy_book.empty()) {
                auto it = buy_book.begin(); // highest price (map is greater<>)
                std::queue<Order>& queue = it->second;
                while (remaining > 0 && !queue.empty()) {
                    Order top = queue.front(); queue.pop();
                    int fill_qty = std::min(remaining, top.quantity - top.filled);
                    remaining -= fill_qty;
                    top.filled += fill_qty;
                    std::cout << "Market SELL filled " << fill_qty << " @ " << it->first << " (OrderID: " << top.order_id << ")\n";
                    if (top.filled < top.quantity) {
                        queue.push(top);
                    } else {
                        order_index.erase(top.order_id);
                    }
                }
                if (queue.empty()) buy_book.erase(it);
            }
        }
        // Note: If remaining > 0, the market order was not fully filled (book empty)
        return;
    }

    // LIMIT ORDER LOGIC (default)
    if (order.side == Side::BUY) {
        buy_book[order.price].push(order);
    } else {
        sell_book[order.price].push(order);
    }
    order_index[order.order_id] = {order.price, order.side};

    // After every new order, check if any stop/stop-limit orders should be triggered
    // For beginners: If the market price crosses a stop order's price, it becomes active
    std::vector<Order> still_pending;
    for (auto& stop_order : stop_orders) {
        bool trigger = false;
        if (!stop_order.triggered) {
            if (stop_order.side == Side::BUY) {
                // Buy stop triggers if best ask >= stop_price
                if (!sell_book.empty() && sell_book.begin()->first >= stop_order.stop_price) trigger = true;
            } else {
                // Sell stop triggers if best bid <= stop_price
                if (!buy_book.empty() && buy_book.begin()->first <= stop_order.stop_price) trigger = true;
            }
        }
        if (trigger) {
            std::cout << "Stop order triggered (OrderID: " << stop_order.order_id << ")\n";
            Order active = stop_order;
            active.triggered = true;
            // STOP becomes MARKET, STOP_LIMIT becomes LIMIT
            if (active.type == OrderType::STOP) {
                active.type = OrderType::MARKET;
            } else if (active.type == OrderType::STOP_LIMIT) {
                active.type = OrderType::LIMIT;
            }
            add_order(active); // Recursively add as active order
        } else {
            still_pending.push_back(stop_order);
        }
    }
    stop_orders = std::move(still_pending);
}

// Cancel an order by ID. Handles both active and pending stop/stop-limit orders.
void OrderBook::cancel_order(int order_id) {
    lock_guard<recursive_mutex> lock(book_mutex);

    // First, try to remove from active order books
    if (order_index.find(order_id) != order_index.end()) {
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
        std::cout << "Order " << order_id << " canceled from active book.\n";
        return;
    }

    // Next, try to remove from pending stop/stop-limit orders
    auto it = std::remove_if(stop_orders.begin(), stop_orders.end(), [order_id](const Order& o) {
        return o.order_id == order_id;
    });
    if (it != stop_orders.end()) {
        stop_orders.erase(it, stop_orders.end());
        std::cout << "Order " << order_id << " canceled from pending stop orders.\n";
    }
}

// Modify an order by ID. Handles both active and pending stop/stop-limit orders.
void OrderBook::modify_order(int order_id, double new_price, int new_qty) {
    lock_guard<recursive_mutex> lock(book_mutex);

    // First, try to modify in active order books
    if (order_index.find(order_id) != order_index.end()) {
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
            std::cout << "Order " << order_id << " modified in active book.\n";
        }
        return;
    }

    // Next, try to modify in pending stop/stop-limit orders
    for (auto& o : stop_orders) {
        if (o.order_id == order_id) {
            o.price = new_price; // For stop-limit, this is the new limit price
            o.quantity = new_qty;
            std::cout << "Order " << order_id << " modified in pending stop orders.\n";
            break;
        }
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
