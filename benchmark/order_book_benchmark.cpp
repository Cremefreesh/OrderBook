#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <new>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

#include "order_book.hpp"


// =============================================
// GLOBAL ALLOCATION INSTRUMENTATION
// =============================================

static bool g_countAllocations = false;

static std::uint64_t g_allocationCount = 0;
static std::uint64_t g_deallocationCount = 0;
static std::uint64_t g_allocatedBytes = 0;


// Override global operator new.
//
// Most standard-library heap allocations eventually
// come through this function.
void* operator new(std::size_t size) {

    if (g_countAllocations) {

        ++g_allocationCount;

        g_allocatedBytes +=
            static_cast<std::uint64_t>(size);
    }


    void* memory =
        std::malloc(size);


    if (memory == nullptr) {
        throw std::bad_alloc();
    }


    return memory;
}


void operator delete(void* memory) noexcept {

    if (g_countAllocations) {
        ++g_deallocationCount;
    }


    std::free(memory);
}


void* operator new[](std::size_t size) {

    if (g_countAllocations) {

        ++g_allocationCount;

        g_allocatedBytes +=
            static_cast<std::uint64_t>(size);
    }


    void* memory =
        std::malloc(size);


    if (memory == nullptr) {
        throw std::bad_alloc();
    }


    return memory;
}


void operator delete[](void* memory) noexcept {

    if (g_countAllocations) {
        ++g_deallocationCount;
    }


    std::free(memory);
}


// Sized delete overloads.
//
// Modern compilers may choose these forms when
// the object's size is known at deletion time.
void operator delete(
    void* memory,
    std::size_t
) noexcept {

    if (g_countAllocations) {
        ++g_deallocationCount;
    }


    std::free(memory);
}


void operator delete[](
    void* memory,
    std::size_t
) noexcept {

    if (g_countAllocations) {
        ++g_deallocationCount;
    }


    std::free(memory);
}


// =============================================
// BENCHMARK TYPES
// =============================================

using Clock =
    std::chrono::steady_clock;

using Nanoseconds =
    std::chrono::nanoseconds;


struct BenchmarkResult {
    std::string name;

    std::size_t operations;

    double throughput;

    double averageLatency;

    std::uint64_t p50;
    std::uint64_t p95;
    std::uint64_t p99;
    std::uint64_t p999;
    std::uint64_t maximum;

    std::uint64_t allocations;
    std::uint64_t deallocations;
    std::uint64_t allocatedBytes;

    double allocationsPerOperation;
};


// =============================================
// PERCENTILE
// =============================================

std::uint64_t percentile(
    const std::vector<std::uint64_t>& sortedSamples,
    double percentileValue
) {

    if (sortedSamples.empty()) {
        return 0;
    }


    std::size_t index =
        static_cast<std::size_t>(
            percentileValue *
            static_cast<double>(
                sortedSamples.size() - 1
            )
        );


    return sortedSamples[index];
}


// =============================================
// RESULT CALCULATION
// =============================================

BenchmarkResult calculateResult(
    const std::string& name,
    std::vector<std::uint64_t> latencies,
    std::uint64_t totalNanoseconds,
    std::uint64_t allocations,
    std::uint64_t deallocations,
    std::uint64_t allocatedBytes
) {

    std::sort(
        latencies.begin(),
        latencies.end()
    );


    std::uint64_t totalLatency =
        std::accumulate(
            latencies.begin(),
            latencies.end(),
            std::uint64_t{0}
        );


    double averageLatency =
        static_cast<double>(
            totalLatency
        )
        /
        static_cast<double>(
            latencies.size()
        );


    double totalSeconds =
        static_cast<double>(
            totalNanoseconds
        )
        /
        1'000'000'000.0;


    double throughput =
        static_cast<double>(
            latencies.size()
        )
        /
        totalSeconds;


    double allocationsPerOperation =
        static_cast<double>(
            allocations
        )
        /
        static_cast<double>(
            latencies.size()
        );


    return {
        name,
        latencies.size(),
        throughput,
        averageLatency,

        percentile(
            latencies,
            0.50
        ),

        percentile(
            latencies,
            0.95
        ),

        percentile(
            latencies,
            0.99
        ),

        percentile(
            latencies,
            0.999
        ),

        latencies.back(),

        allocations,
        deallocations,
        allocatedBytes,

        allocationsPerOperation
    };
}


// =============================================
// RESULT OUTPUT
// =============================================

void printResult(
    const BenchmarkResult& result
) {

    std::cout
        << "\n"
        << result.name
        << "\n";


    std::cout
        << "------------------------------\n";


    std::cout
        << "Operations: "
        << result.operations
        << '\n';


    std::cout
        << "Throughput: "
        << result.throughput
        << " operations/sec\n";


    std::cout
        << "Average latency: "
        << result.averageLatency
        << " ns\n";


    std::cout
        << "p50 latency: "
        << result.p50
        << " ns\n";


    std::cout
        << "p95 latency: "
        << result.p95
        << " ns\n";


    std::cout
        << "p99 latency: "
        << result.p99
        << " ns\n";


    std::cout
        << "p99.9 latency: "
        << result.p999
        << " ns\n";


    std::cout
        << "Maximum latency: "
        << result.maximum
        << " ns\n";


    std::cout
        << "\nMemory activity\n";


    std::cout
        << "Allocations: "
        << result.allocations
        << '\n';


    std::cout
        << "Deallocations: "
        << result.deallocations
        << '\n';


    std::cout
        << "Allocated bytes: "
        << result.allocatedBytes
        << '\n';


    std::cout
        << "Allocations / operation: "
        << result.allocationsPerOperation
        << '\n';
}


// =============================================
// RESET ALLOCATION COUNTERS
// =============================================

void resetAllocationCounters() {

    g_allocationCount = 0;

    g_deallocationCount = 0;

    g_allocatedBytes = 0;
}


// =============================================
// RESTING INSERT BENCHMARK
// =============================================

BenchmarkResult benchmarkRestingInsert(
    std::size_t numOrders
) {

    // We know approximately how many active orders
    // this workload will contain, so reserve the
    // unordered_map upfront.
    OrderBook book(
        numOrders + 1000
    );


    OrderId nextOrderId = 1;


    // Preload 1000 asks.
    //
    // These are outside the measured region.
    for (
        Price price = 10000;
        price < 10100;
        ++price
    ) {

        for (int i = 0; i < 10; ++i) {

            book.addLimitOrder({
                nextOrderId++,
                price,
                100,
                Side::Sell
            });
        }
    }


    std::vector<std::uint64_t> latencies;

    latencies.reserve(
        numOrders
    );


    resetAllocationCounters();


    auto benchmarkStart =
        Clock::now();


    for (
        std::size_t i = 0;
        i < numOrders;
        ++i
    ) {

        Order order {
            nextOrderId++,
            9990,
            10,
            Side::Buy
        };


        g_countAllocations = true;


        auto start =
            Clock::now();


        auto result =
            book.addLimitOrder(order);


        auto end =
            Clock::now();


        g_countAllocations = false;


        auto latency =
            std::chrono::duration_cast<
                Nanoseconds
            >(
                end - start
            ).count();


        latencies.push_back(
            static_cast<std::uint64_t>(
                latency
            )
        );


        if (!result.accepted()) {

            std::cerr
                << "Order rejected during "
                << "resting insert benchmark\n";

            std::exit(1);
        }
    }


    auto benchmarkEnd =
        Clock::now();


    auto totalNanoseconds =
        std::chrono::duration_cast<
            Nanoseconds
        >(
            benchmarkEnd -
            benchmarkStart
        ).count();


    return calculateResult(
        "Resting Limit Insert",
        std::move(latencies),

        static_cast<std::uint64_t>(
            totalNanoseconds
        ),

        g_allocationCount,
        g_deallocationCount,
        g_allocatedBytes
    );
}


// =============================================
// EXACT MATCH BENCHMARK
// =============================================

BenchmarkResult benchmarkExactMatch(
    std::size_t numOrders
) {

    OrderBook book(
        numOrders
    );


    OrderId nextOrderId = 1;


    // Build a large FIFO queue of resting sells.
    //
    // Again: setup is outside the measured region.
    for (
        std::size_t i = 0;
        i < numOrders;
        ++i
    ) {

        book.addLimitOrder({
            nextOrderId++,
            10000,
            10,
            Side::Sell
        });
    }


    std::vector<std::uint64_t> latencies;

    latencies.reserve(
        numOrders
    );


    resetAllocationCounters();


    auto benchmarkStart =
        Clock::now();


    for (
        std::size_t i = 0;
        i < numOrders;
        ++i
    ) {

        Order order {
            nextOrderId++,
            10000,
            10,
            Side::Buy
        };


        g_countAllocations = true;


        auto start =
            Clock::now();


        auto result =
            book.addLimitOrder(order);


        auto end =
            Clock::now();


        g_countAllocations = false;


        auto latency =
            std::chrono::duration_cast<
                Nanoseconds
            >(
                end - start
            ).count();


        latencies.push_back(
            static_cast<std::uint64_t>(
                latency
            )
        );


        if (
            !result.accepted() ||
            result.trades.size() != 1
        ) {

            std::cerr
                << "Unexpected result during "
                << "exact match benchmark\n";

            std::exit(1);
        }
    }


    auto benchmarkEnd =
        Clock::now();


    auto totalNanoseconds =
        std::chrono::duration_cast<
            Nanoseconds
        >(
            benchmarkEnd -
            benchmarkStart
        ).count();


    return calculateResult(
        "Exact Match",
        std::move(latencies),

        static_cast<std::uint64_t>(
            totalNanoseconds
        ),

        g_allocationCount,
        g_deallocationCount,
        g_allocatedBytes
    );
}


// =============================================
// MAIN
// =============================================

int main() {

    constexpr std::size_t NUM_ORDERS =
        100000;


    auto restingInsert =
        benchmarkRestingInsert(
            NUM_ORDERS
        );


    auto exactMatch =
        benchmarkExactMatch(
            NUM_ORDERS
        );


    printResult(
        restingInsert
    );


    printResult(
        exactMatch
    );


    return 0;
}