#pragma once

#include <string>

enum class Side { BUY, SELL };
enum class OrderType { LIMIT, MARKET };
enum class OrderStatus { OPEN, PARTIALLY_FILLED, FILLED, CANCELED };

struct Order {
    int order_id;
    long long timestamp;
    Side side;
    OrderType type;
    double price;
    int quantity;
    int filled = 0;
    OrderStatus status;

    Order(
        int id,
        long long ts,
        Side s,
        OrderType t,
        double p,
        int qty
    ) : order_id(id), timestamp(ts), side(s), type(t), price(p), quantity(qty), status(OrderStatus::OPEN) {}
};
