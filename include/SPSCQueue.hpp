#pragma once

#include <array>
#include <atomic>
#include <cstddef>

template <typename T, std::size_t Capacity>
class SPSCQueue {
public:
    SPSCQueue() = default;

    bool push(const T& value) {
        // TODO
        return false;
    }

    bool pop(T& value) {
        // TODO
        return false;
    }

    bool empty() const {
        return true;
    }

private:
    std::array<T, Capacity> buffer_{};


    //head = next element the consumer reads
    //tail = next position the producer writes
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
};