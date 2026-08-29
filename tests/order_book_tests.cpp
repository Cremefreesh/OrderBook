#include <cassert>
#include <iostream>

#include "order_book.hpp"


void testNonCrossingOrders() {
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
        100,
        Side::Sell
    });

    assert(book.bestBid() == 10000);
    assert(book.bestAsk() == 10005);
}


void testExactFill() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        50,
        Side::Sell
    });

    auto trades = book.addLimitOrder({
        2,
        10000,
        50,
        Side::Buy
    });

    assert(trades.size() == 1);

    assert(trades[0].buyOrderId == 2);
    assert(trades[0].sellOrderId == 1);
    assert(trades[0].price == 10000);
    assert(trades[0].quantity == 50);

    assert(book.bestBid() == 0);
    assert(book.bestAsk() == 0);
}


void testPartialFill() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10005,
        50,
        Side::Sell
    });

    auto trades = book.addLimitOrder({
        2,
        10005,
        100,
        Side::Buy
    });

    assert(trades.size() == 1);
    assert(trades[0].quantity == 50);

    assert(book.bestBid() == 10005);
    assert(book.bestAsk() == 0);
}


void testPriceTimePriority() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10005,
        30,
        Side::Sell
    });

    book.addLimitOrder({
        2,
        10005,
        30,
        Side::Sell
    });

    auto trades = book.addLimitOrder({
        3,
        10005,
        40,
        Side::Buy
    });

    assert(trades.size() == 2);

    assert(trades[0].sellOrderId == 1);
    assert(trades[0].quantity == 30);

    assert(trades[1].sellOrderId == 2);
    assert(trades[1].quantity == 10);
}


void testSweepMultiplePriceLevels() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10001,
        20,
        Side::Sell
    });

    book.addLimitOrder({
        2,
        10002,
        20,
        Side::Sell
    });

    book.addLimitOrder({
        3,
        10003,
        20,
        Side::Sell
    });

    auto trades = book.addLimitOrder({
        4,
        10003,
        50,
        Side::Buy
    });

    assert(trades.size() == 3);

    assert(trades[0].price == 10001);
    assert(trades[0].quantity == 20);

    assert(trades[1].price == 10002);
    assert(trades[1].quantity == 20);

    assert(trades[2].price == 10003);
    assert(trades[2].quantity == 10);

    assert(book.bestAsk() == 10003);
}


void testCancelExistingOrder() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        100,
        Side::Buy
    });

    bool cancelled = book.cancelOrder(1);

    assert(cancelled);
    assert(book.bestBid() == 0);
}


void testCancelNonexistentOrder() {
    OrderBook book;

    bool cancelled = book.cancelOrder(999);

    assert(!cancelled);
}


void testCancelOneOrderAtSharedPriceLevel() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        100,
        Side::Buy
    });

    book.addLimitOrder({
        2,
        10000,
        100,
        Side::Buy
    });

    bool cancelled = book.cancelOrder(1);

    assert(cancelled);

    assert(book.bestBid() == 10000);
}


int main() {
    testNonCrossingOrders();
    testExactFill();
    testPartialFill();
    testPriceTimePriority();
    testSweepMultiplePriceLevels();
    testCancelExistingOrder();
    testCancelNonexistentOrder();
    testCancelOneOrderAtSharedPriceLevel();

    std::cout << "All order book tests passed.\n";

    return 0;
}