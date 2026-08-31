cmake_minimum_required(VERSION 3.20)

project(OrderBook)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(orderbook
    src/main.cpp
    src/order_book.cpp
)

target_include_directories(
    orderbook
    PRIVATE
    include
)


add_executable(orderbook_tests
    tests/order_book_tests.cpp
    src/order_book.cpp
)

target_include_directories(
    orderbook_tests
    PRIVATE
    include
)


add_executable(orderbook_benchmark
    benchmark/order_book_benchmark.cpp
    src/order_book.cpp
)

target_include_directories(
    orderbook_benchmark
    PRIVATE
    include
)