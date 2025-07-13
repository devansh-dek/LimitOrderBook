#pragma once
#include "order.hpp"
#include "order_book.hpp"

class Matcher {
public:
    void match_order(Order& incoming_order, OrderBook& book);
};
