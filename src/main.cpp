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

    std::cout
        << "Best bid: "
        << book.bestBid()
        << '\n';

    std::cout
        << "Best ask: "
        << book.bestAsk()
        << '\n';


    OrderResult result =
        book.addLimitOrder({
            3,
            10005,
            100,
            Side::Buy
        });


    for (const Trade& trade : result.trades) {

        std::cout
            << "Trade: buy="
            << trade.buyOrderId

            << " sell="
            << trade.sellOrderId

            << " price="
            << trade.price

            << " quantity="
            << trade.quantity

            << '\n';
    }


    std::cout << "\nAfter crossing order:\n";

    std::cout
        << "Best bid: "
        << book.bestBid()
        << '\n';

    std::cout
        << "Best ask: "
        << book.bestAsk()
        << '\n';


    bool cancelled =
        book.cancelOrder(3);


    std::cout
        << "\nCancel order 3: "
        << (cancelled ? "success" : "failed")
        << '\n';


    std::cout
        << "Best bid after cancel: "
        << book.bestBid()
        << '\n';


    return 0;
}