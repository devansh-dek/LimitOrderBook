#include <iostream>
#include <thread>
#include <vector>
#include <random>
#include <chrono>
#include "order.hpp"
#include "order_book.hpp"
#include "matcher.hpp"
#include "thread_safe_queue.hpp"

OrderBook book;
Matcher matcher;
ThreadSafeQueue<Order> order_queue;

std::atomic<int> global_order_id = 1;

// 🧠 Producer: randomly generates orders
void producer_func(int trader_id) {
    std::default_random_engine rng(std::random_device{}());
    std::uniform_int_distribution<int> qty_dist(1, 50);
    std::uniform_real_distribution<double> price_dist(99.0, 101.0);
    std::uniform_int_distribution<int> side_dist(0, 1);

    for (int i = 0; i < 10; ++i) {
        Side side = (side_dist(rng) == 0) ? Side::BUY : Side::SELL;
        double price = price_dist(rng);
        int qty = qty_dist(rng);

        Order order(global_order_id++, std::chrono::system_clock::now().time_since_epoch().count(), side, OrderType::LIMIT, price, qty);
        order_queue.push(order);
        std::this_thread::sleep_for(std::chrono::milliseconds(100 + trader_id * 10));
    }
}

// ⚙️ Consumer: matches orders
void matcher_func() {
    while (true) {
        Order incoming = order_queue.pop();
        if (incoming.order_id == -1) break; // Poison pill to exit
        matcher.match_order(incoming, book);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    const int NUM_PRODUCERS = 4;

    // 🧵 Spawn matcher
    std::thread matcher_thread(matcher_func);

    // 🧵 Spawn traders
    std::vector<std::thread> producers;
    for (int i = 0; i < NUM_PRODUCERS; ++i) {
        producers.emplace_back(producer_func, i + 1);
    }

    for (auto& t : producers) t.join();

    // Let matcher catch up
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Send poison pill to stop matcher
    order_queue.push(Order(-1, 0, Side::BUY, OrderType::LIMIT, 0.0, 0));
    matcher_thread.join();

    book.print_top_levels();

    return 0;
}
