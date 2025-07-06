#include "matcher.hpp"
#include "bits/stdc++.h"

int main() {
    // OrderBook book;
    // Matcher matcher;

    // Add existing SELL orders
    // book.add_order(Order(2, 1002, Side::SELL, OrderType::LIMIT, 100.0, 50));
    // book.add_order(Order(3, 1003, Side::SELL, OrderType::LIMIT, 100.5, 30));

    // // Incoming BUY order
    // Order incoming(10, 1010, Side::BUY, OrderType::LIMIT, 100.5, 60);
    // matcher.match_order(incoming, book);

    OrderBook book;

    std::cout << "Adding order 1" << std::endl;
    book.add_order(Order(1, 101, Side::BUY, OrderType::LIMIT, 100.5, 20));
    std::cout << "Adding order 2" << std::endl;
    book.add_order(Order(2, 102, Side::BUY, OrderType::LIMIT, 100.0, 10));
    std::cout << "Canceling order 2" << std::endl;
    book.cancel_order(2); // Cancel order 2
    std::cout << "Modifying order 1" << std::endl;
    book.modify_order(1, 101.0, 50); // Modify order 1
    std::cout << "Printing book" << std::endl;
    book.print_top_levels();
    std::cout << "Done" << std::endl;
    return 0;
}
