#include "order_book.hpp"
#include <cassert>
#include <iostream>

// Simple test for market, stop, and stop-limit orders
int main() {
    OrderBook ob;
    long long ts = 1;

    // Add some limit orders to create a book
    ob.add_order(Order(1, ts++, Side::SELL, OrderType::LIMIT, 100.0, 10)); // Sell 10 @ 100
    ob.add_order(Order(2, ts++, Side::SELL, OrderType::LIMIT, 101.0, 10)); // Sell 10 @ 101
    ob.add_order(Order(3, ts++, Side::BUY, OrderType::LIMIT, 99.0, 10));   // Buy 10 @ 99
    ob.add_order(Order(4, ts++, Side::BUY, OrderType::LIMIT, 98.0, 10));   // Buy 10 @ 98

    // Test market buy (should fill at 100)
    ob.add_order(Order(10, ts++, Side::BUY, OrderType::MARKET, 0.0, 5));
    // Test market sell (should fill at 99)
    ob.add_order(Order(11, ts++, Side::SELL, OrderType::MARKET, 0.0, 5));

    // Test stop buy (should trigger when best ask >= 102)
    ob.add_order(Order(20, ts++, Side::BUY, OrderType::STOP, 0.0, 5, 102.0));
    // Add a sell limit at 102 to trigger stop buy
    ob.add_order(Order(21, ts++, Side::SELL, OrderType::LIMIT, 102.0, 5));

    // Test stop-limit sell (should trigger when best bid <= 97)
    ob.add_order(Order(30, ts++, Side::SELL, OrderType::STOP_LIMIT, 97.0, 5, 97.0));
    // Add a buy limit at 97 to trigger stop-limit sell
    ob.add_order(Order(31, ts++, Side::BUY, OrderType::LIMIT, 97.0, 5));

    // Test cancel and modify for stop order
    ob.add_order(Order(40, ts++, Side::BUY, OrderType::STOP, 0.0, 5, 105.0));
    ob.cancel_order(40); // Should remove from stop_orders
    ob.add_order(Order(41, ts++, Side::SELL, OrderType::STOP_LIMIT, 96.0, 5, 96.0));
    ob.modify_order(41, 95.0, 10); // Should update price and quantity in stop_orders

    std::cout << "All tests ran. Please check output for correct triggering and fills.\n";
    return 0;
}
