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



    
    bool pop(T& value) {
        const auto head = head_.load(std::memory_order_relaxed);
        const auto next_head = (head + 1) % Capacity;

        if (head == tail_.load(std::memory_order_acquire)) {
            return false;
        }

        value = buffer_[head];
        head_.store(next_head, std::memory_order_release);
        return true;
    }

    
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


A Single-Producer Single-Consumer (SPSC) lock-free ring buffer is a high-performance data 
structure used to pass data safely between two threads without using traditional 
locking primitives like mutexes.
How It Works Ring Buffer: 
A fixed-size array functioning as a circular queue where write and read indices wrap around to the beginning when 
 reaching the end.
Single-Producer Single-Consumer (SPSC): Exactly one thread writes data (the producer) 
 and exactly one thread reads data (the consumer).
Lock-Free: Coordination happens via 
 atomic index variables and memory barriers rather than operating system blocking locks

 SPSCQueue<T, Capacity>

✓ fixed-capacity
✓ preallocated storage
✓ no mutex
✓ no allocation during push/pop
✓ atomic producer/consumer synchronization
✓ acquire/release ordering
✓ circular storage
✓ FIFO
✓ cache-line-separated indices

*/