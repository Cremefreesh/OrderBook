#include <iostream>

#include "order_book.hpp"

int main() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        100,
        Side::Buy
    });

    book.addLimitOrder({
        2,
        10005,
        50,
        Side::Sell
    });

    std::cout << "Before crossing order:\n";
    std::cout << "Best bid: " << book.bestBid() << '\n';
    std::cout << "Best ask: " << book.bestAsk() << '\n';

    book.addLimitOrder({
        3,
        10005,
        100,
        Side::Buy
    });

    std::cout << "\nAfter crossing order:\n";
    std::cout << "Best bid: " << book.bestBid() << '\n';
    std::cout << "Best ask: " << book.bestAsk() << '\n';

    return 0;


    auto trades = book.addLimitOrder({
        3,
        10005,
        100,
        Side::Buy
    });

    for (const Trade& trade : trades) {
        std::cout
            << "Trade: buy=" << trade.buyOrderId
            << " sell=" << trade.sellOrderId
            << " price=" << trade.price
            << " quantity=" << trade.quantity
            << '\n';
    }


}