#include "SPSCQueue.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <thread>

void test_basic_queue() {
    SPSCQueue<int, 4> queue;

    assert(queue.empty());

    assert(queue.push(10));
    assert(queue.push(20));
    assert(queue.push(30));

    // Capacity 4 currently gives us 3 usable slots
    assert(!queue.push(40));

    int value;

    assert(queue.pop(value));
    assert(value == 10);

    assert(queue.pop(value));
    assert(value == 20);

    assert(queue.pop(value));
    assert(value == 30);

    assert(!queue.pop(value));
    assert(queue.empty());

    std::cout << "Basic test passed!\n";
}


void test_concurrent_queue() {
    constexpr std::size_t QUEUE_SIZE = 1024;
    constexpr std::uint64_t NUM_ITEMS = 1'000'000;

    SPSCQueue<std::uint64_t, QUEUE_SIZE> queue;

    std::thread producer([&]() {
        for (std::uint64_t i = 0; i < NUM_ITEMS; ++i) {
            while (!queue.push(i)) {
                // spin while full
            }
        }
    });

    std::thread consumer([&]() {
        for (std::uint64_t expected = 0;
             expected < NUM_ITEMS;
             ++expected) {

            std::uint64_t value;

            while (!queue.pop(value)) {
                // spin while empty
            }

            assert(value == expected);
        }
    });

    producer.join();
    consumer.join();

    assert(queue.empty());

    std::cout << "Concurrent test passed!\n";
}


int main() {
    test_basic_queue();
    test_concurrent_queue();

    std::cout << "All SPSC queue tests passed!\n";

    return 0;
}