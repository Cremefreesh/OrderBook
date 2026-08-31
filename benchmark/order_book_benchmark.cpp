#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <vector>

#include "order_book.hpp"


using Clock =
    std::chrono::steady_clock;

using Nanoseconds =
    std::chrono::nanoseconds;


std::uint64_t percentile(
    std::vector<std::uint64_t> samples,
    double percentileValue
) {

    if (samples.empty()) {
        return 0;
    }


    std::sort(
        samples.begin(),
        samples.end()
    );


    std::size_t index =
        static_cast<std::size_t>(
            percentileValue *
            static_cast<double>(
                samples.size() - 1
            )
        );


    return samples[index];
}


int main() {

    constexpr std::size_t NUM_ORDERS =
        100000;


    OrderBook book;


    // -----------------------------------------
    // PRELOAD ASK LIQUIDITY
    // -----------------------------------------

    OrderId nextOrderId = 1;


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


    // -----------------------------------------
    // LATENCY STORAGE
    // -----------------------------------------

    std::vector<std::uint64_t>
        latencies;


    latencies.reserve(
        NUM_ORDERS
    );


    // -----------------------------------------
    // BENCHMARK START
    // -----------------------------------------

    auto benchmarkStart =
        Clock::now();


    for (
        std::size_t i = 0;
        i < NUM_ORDERS;
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
                << "Order rejected during benchmark\n";

            return 1;
        }
    }


    auto benchmarkEnd =
        Clock::now();


    // -----------------------------------------
    // TOTAL TIME
    // -----------------------------------------

    auto totalNanoseconds =
        std::chrono::duration_cast<
            Nanoseconds
        >(
            benchmarkEnd -
            benchmarkStart
        ).count();


    double totalSeconds =
        static_cast<double>(
            totalNanoseconds
        ) / 1'000'000'000.0;


    double throughput =
        static_cast<double>(
            NUM_ORDERS
        ) / totalSeconds;


    // -----------------------------------------
    // AVERAGE LATENCY
    // -----------------------------------------

    std::uint64_t totalLatency =
        std::accumulate(
            latencies.begin(),
            latencies.end(),
            std::uint64_t{0}
        );


    double averageLatency =
        static_cast<double>(
            totalLatency
        ) /
        static_cast<double>(
            latencies.size()
        );


    // -----------------------------------------
    // REPORT
    // -----------------------------------------

    std::cout
        << "\nOrder Book Benchmark\n"
        << "--------------------\n";


    std::cout
        << "Orders: "
        << NUM_ORDERS
        << '\n';


    std::cout
        << "Throughput: "
        << throughput
        << " orders/sec\n";


    std::cout
        << "Average latency: "
        << averageLatency
        << " ns\n";


    std::cout
        << "p50 latency: "
        << percentile(
            latencies,
            0.50
        )
        << " ns\n";


    std::cout
        << "p95 latency: "
        << percentile(
            latencies,
            0.95
        )
        << " ns\n";


    std::cout
        << "p99 latency: "
        << percentile(
            latencies,
            0.99
        )
        << " ns\n";


    std::cout
        << "p99.9 latency: "
        << percentile(
            latencies,
            0.999
        )
        << " ns\n";


    std::cout
        << "Maximum latency: "
        << *std::max_element(
            latencies.begin(),
            latencies.end()
        )
        << " ns\n";


    return 0;
}