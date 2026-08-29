#include <iostream>

#include "order_book.hpp"

int main() {
    OrderBook book;

    Order buy1 {
        1,
        10000,
        100,
        Side::Buy
    };

    Order buy2 {
        2,
        9999,
        50,
        Side::Buy
    };

    Order sell1 {
        3,
        10005,
        75,
        Side::Sell
    };

    book.addLimitOrder(buy1);
    book.addLimitOrder(buy2);
    book.addLimitOrder(sell1);

    std::cout << "Best bid: " << book.bestBid() << '\n';
    std::cout << "Best ask: " << book.bestAsk() << '\n';

    return 0;
}