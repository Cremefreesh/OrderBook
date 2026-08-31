#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <string>
#include <vector>

#include "order_book.hpp"


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
};


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


BenchmarkResult calculateResult(
    const std::string& name,
    std::vector<std::uint64_t> latencies,
    std::uint64_t totalNanoseconds
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


    return {
        name,
        latencies.size(),
        throughput,
        averageLatency,
        percentile(latencies, 0.50),
        percentile(latencies, 0.95),
        percentile(latencies, 0.99),
        percentile(latencies, 0.999),
        latencies.back()
    };
}


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
}


// =============================================
// RESTING INSERT BENCHMARK
// =============================================

BenchmarkResult benchmarkRestingInsert(
    std::size_t numOrders
) {

    OrderBook book(
        numOrders + 1000
    );


    OrderId nextOrderId = 1;


    // Preload asks so the opposite side exists,
    // but incoming buys will not cross.
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

    latencies.reserve(numOrders);


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


        auto start =
            Clock::now();


        auto result =
            book.addLimitOrder(order);


        auto end =
            Clock::now();


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
        )
    );
}


// =============================================
// EXACT MATCH BENCHMARK
// =============================================

BenchmarkResult benchmarkExactMatch(
    std::size_t numOrders
) {

    OrderBook book;


    OrderId nextOrderId = 1;


    // Build a queue of resting sell orders.
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

    latencies.reserve(numOrders);


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


        auto start =
            Clock::now();


        auto result =
            book.addLimitOrder(order);


        auto end =
            Clock::now();


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
        )
    );
}


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