#include "matcher.hpp"

int main() {
    OrderBook book;
    Matcher matcher;

    // Add existing SELL orders
    book.add_order(Order(2, 1002, Side::SELL, OrderType::LIMIT, 100.0, 50));
    book.add_order(Order(3, 1003, Side::SELL, OrderType::LIMIT, 100.5, 30));

    // Incoming BUY order
    Order incoming(10, 1010, Side::BUY, OrderType::LIMIT, 100.5, 60);
    matcher.match_order(incoming, book);

    book.print_top_levels();
    return 0;
}
