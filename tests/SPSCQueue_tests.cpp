#include "SPSCQueue.hpp"

#include <iostream>

int main() {
    SPSCQueue<int, 4> queue;

    std::cout << queue.empty() << '\n';

    queue.push(10);
    queue.push(20);
    queue.push(30);

    // This should fail because our usable
    // capacity is currently Capacity - 1.
    std::cout << queue.push(40) << '\n';

    int value;

    while (queue.pop(value)) {
        std::cout << value << '\n';
    }
}