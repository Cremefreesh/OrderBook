#include <iostream>

#include "order_book.hpp"

int main() {
    OrderBook book;

    std::cout << "Best bid: " << book.bestBid() << '\n';
    std::cout << "Best ask: " << book.bestAsk() << '\n';

    return 0;
}