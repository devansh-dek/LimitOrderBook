#include <iostream>
#include "order.hpp"

int main() {
    Order order1(1, 100000, Side::BUY, OrderType::LIMIT, 99.5, 100);
    std::cout << "Order created with ID: " << order1.order_id << std::endl;
    return 0;
}
