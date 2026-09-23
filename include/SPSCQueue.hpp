#pragma once

#include <array>
#include <atomic>
#include <cstddef>

template <typename T, std::size_t Capacity>
class SPSCQueue {
public:
    SPSCQueue() = default;

    bool push(const T& value) {
        const auto tail = tail_.load(std::memory_order_relaxed);
        const auto next_tail = (tail + 1) % Capacity;

        //full queue, cannot push
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false;
        }

        buffer_[tail] = value;

        tail_.store(next_tail, std::memory_order_release);

        return true;
    }



    //autofilled by vs code --> check
    bool pop(T& value) {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next_head = (head + 1) % Capacity;

        if (next_head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        value = buffer_[head];
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    //autofilled by vs code --> check
    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }

private:
    std::array<T, Capacity> buffer_{};


    //head = next element the consumer reads
    //tail = next position the producer writes
    //head for removing, tail for inserting
    std::atomic<std::size_t> head_{0};
    std::atomic<std::size_t> tail_{0};
};


//             WRITES              READS

//head_        consumer            producer

//tail_        producer            consumer


/*
THREAD A                         THREAD B

write useful data
      │
      ▼
release STORE ───────────────► acquire LOAD
                                    │
                                    ▼
                             safely observe
                             preceding effects
*/