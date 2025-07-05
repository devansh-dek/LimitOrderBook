#include <iostream>
#include "order.hpp"
#include "order_book.hpp"

int main() {
    OrderBook book;
    book.add_order(Order(1, 1001, Side::BUY, OrderType::LIMIT, 100.5, 10));
    book.add_order(Order(2, 1002, Side::SELL, OrderType::LIMIT, 101.0, 5));
    book.add_order(Order(3, 1003, Side::BUY, OrderType::LIMIT, 100.5, 20));
    book.add_order(Order(4, 1004, Side::SELL, OrderType::LIMIT, 100.8, 15));

    book.print_top_levels();
    return 0;
}
