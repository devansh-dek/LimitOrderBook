#pragma once

#include <string>

enum class Side { BUY, SELL };
// OrderType now supports LIMIT, MARKET, STOP, and STOP_LIMIT orders
enum class OrderType { LIMIT, MARKET, STOP, STOP_LIMIT };
enum class OrderStatus { OPEN, PARTIALLY_FILLED, FILLED, CANCELED };


// The Order struct represents a single order in the order book.
// It now supports stop and stop-limit orders with the stop_price field.
struct Order {
    int order_id;                // Unique order identifier
    long long timestamp;         // Time the order was created
    Side side;                   // BUY or SELL
    OrderType type;              // LIMIT, MARKET, STOP, or STOP_LIMIT
    double price;                // Limit price (for LIMIT/STOP_LIMIT orders)
    int quantity;                // Total quantity requested
    int filled = 0;              // Quantity already filled
    OrderStatus status;          // Current status of the order
    double stop_price = 0.0;     // Stop price (for STOP/STOP_LIMIT orders)
    bool triggered = false;      // True if stop order has been triggered

    // Constructor for LIMIT and MARKET orders
    Order(
        int id,
        long long ts,
        Side s,
        OrderType t,
        double p,
        int qty
    ) : order_id(id), timestamp(ts), side(s), type(t), price(p), quantity(qty), status(OrderStatus::OPEN) {}

    // Constructor for STOP and STOP_LIMIT orders
    Order(
        int id,
        long long ts,
        Side s,
        OrderType t,
        double p,
        int qty,
        double stop_p
    ) : order_id(id), timestamp(ts), side(s), type(t), price(p), quantity(qty), status(OrderStatus::OPEN), stop_price(stop_p), triggered(false) {}
};
