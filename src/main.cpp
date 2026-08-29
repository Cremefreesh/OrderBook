#include <iostream>

#include "order_book.hpp"


int main() {

    OrderBook book;


    // ---------------------------------
    // Add initial resting BUY
    // ---------------------------------

    book.addLimitOrder({
        1,
        10000,
        100,
        Side::Buy
    });


    // ---------------------------------
    // Add initial resting SELL
    // ---------------------------------

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


    // ---------------------------------
    // Submit crossing BUY
    // ---------------------------------

    auto trades = book.addLimitOrder({
        3,
        10005,
        100,
        Side::Buy
    });


    // ---------------------------------
    // Print resulting trades
    // ---------------------------------

    for (const Trade& trade : trades) {

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


    // ---------------------------------
    // Cancel remaining Order 3
    // ---------------------------------

    bool cancelled = book.cancelOrder(3);


    std::cout
        << "\nCancel order 3: "
        << (cancelled ? "success" : "failed")
        << '\n';


    std::cout
        << "Best bid after cancel: "
        << book.bestBid()
        << '\n';


    // ---------------------------------
    // Try cancelling it again
    // ---------------------------------

    bool cancelledAgain = book.cancelOrder(3);

    std::cout
        << "Cancel order 3 again: "
        << (cancelledAgain ? "success" : "failed")
        << '\n';


    return 0;
}