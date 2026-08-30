#include <cassert>
#include <iostream>

#include "order_book.hpp"


void testNonCrossingOrders() {
    OrderBook book;

    auto buyResult = book.addLimitOrder({
        1,
        10000,
        100,
        Side::Buy
    });

    auto sellResult = book.addLimitOrder({
        2,
        10005,
        100,
        Side::Sell
    });

    assert(buyResult.accepted());
    assert(sellResult.accepted());

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

    auto result = book.addLimitOrder({
        2,
        10000,
        50,
        Side::Buy
    });

    assert(result.accepted());
    assert(result.trades.size() == 1);

    assert(result.trades[0].buyOrderId == 2);
    assert(result.trades[0].sellOrderId == 1);
    assert(result.trades[0].price == 10000);
    assert(result.trades[0].quantity == 50);

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

    auto result = book.addLimitOrder({
        2,
        10005,
        100,
        Side::Buy
    });

    assert(result.accepted());

    assert(result.trades.size() == 1);
    assert(result.trades[0].quantity == 50);

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

    auto result = book.addLimitOrder({
        3,
        10005,
        40,
        Side::Buy
    });

    assert(result.trades.size() == 2);

    assert(result.trades[0].sellOrderId == 1);
    assert(result.trades[0].quantity == 30);

    assert(result.trades[1].sellOrderId == 2);
    assert(result.trades[1].quantity == 10);
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

    auto result = book.addLimitOrder({
        4,
        10003,
        50,
        Side::Buy
    });

    assert(result.trades.size() == 3);

    assert(result.trades[0].price == 10001);
    assert(result.trades[0].quantity == 20);

    assert(result.trades[1].price == 10002);
    assert(result.trades[1].quantity == 20);

    assert(result.trades[2].price == 10003);
    assert(result.trades[2].quantity == 10);

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

    bool cancelled =
        book.cancelOrder(1);

    assert(cancelled);
    assert(book.bestBid() == 0);
}


void testCancelNonexistentOrder() {
    OrderBook book;

    assert(!book.cancelOrder(999));
}


void testDuplicateOrderIdRejected() {
    OrderBook book;

    auto firstResult =
        book.addLimitOrder({
            1,
            10000,
            100,
            Side::Buy
        });

    auto duplicateResult =
        book.addLimitOrder({
            1,
            11000,
            100,
            Side::Buy
        });

    assert(firstResult.accepted());

    assert(
        duplicateResult.status ==
        OrderStatus::DuplicateOrderId
    );

    assert(!duplicateResult.accepted());

    assert(book.bestBid() == 10000);
}


void testModifyNonexistentOrder() {
    OrderBook book;

    auto result =
        book.modifyOrder(
            999,
            10000,
            50
        );

    assert(
        result.status ==
        OrderStatus::OrderNotFound
    );

    assert(!result.accepted());
}


void testModifyQuantityToZeroCancels() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        100,
        Side::Buy
    });

    auto result =
        book.modifyOrder(
            1,
            10000,
            0
        );

    assert(result.accepted());

    assert(book.bestBid() == 0);
    assert(!book.cancelOrder(1));
}


void testQuantityDecreaseKeepsPriority() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        100,
        Side::Sell
    });

    book.addLimitOrder({
        2,
        10000,
        100,
        Side::Sell
    });

    auto modifyResult =
        book.modifyOrder(
            1,
            10000,
            50
        );

    assert(modifyResult.accepted());

    auto result =
        book.addLimitOrder({
            3,
            10000,
            50,
            Side::Buy
        });

    assert(result.trades.size() == 1);
    assert(result.trades[0].sellOrderId == 1);
}


void testQuantityIncreaseLosesPriority() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        50,
        Side::Sell
    });

    book.addLimitOrder({
        2,
        10000,
        50,
        Side::Sell
    });

    auto modifyResult =
        book.modifyOrder(
            1,
            10000,
            100
        );

    assert(modifyResult.accepted());

    auto result =
        book.addLimitOrder({
            3,
            10000,
            50,
            Side::Buy
        });

    assert(result.trades.size() == 1);
    assert(result.trades[0].sellOrderId == 2);
}


void testModifyCanGenerateTrades() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10005,
        50,
        Side::Sell
    });

    book.addLimitOrder({
        2,
        10000,
        50,
        Side::Buy
    });

    auto result =
        book.modifyOrder(
            2,
            10005,
            50
        );

    assert(result.accepted());

    assert(result.trades.size() == 1);

    assert(result.trades[0].buyOrderId == 2);
    assert(result.trades[0].sellOrderId == 1);
    assert(result.trades[0].price == 10005);
    assert(result.trades[0].quantity == 50);

    assert(book.bestBid() == 0);
    assert(book.bestAsk() == 0);
}


// =============================================
// MARKET TESTS
// =============================================

void testMarketBuyConsumesBestAsk() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10005,
        50,
        Side::Sell
    });

    auto result =
        book.addMarketOrder(
            2,
            50,
            Side::Buy
        );

    assert(result.accepted());
    assert(result.trades.size() == 1);

    assert(result.trades[0].price == 10005);
    assert(result.trades[0].quantity == 50);

    assert(book.bestAsk() == 0);
}


void testMarketSellConsumesBestBid() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10000,
        40,
        Side::Buy
    });

    auto result =
        book.addMarketOrder(
            2,
            40,
            Side::Sell
        );

    assert(result.accepted());
    assert(result.trades.size() == 1);

    assert(result.trades[0].price == 10000);
    assert(result.trades[0].quantity == 40);

    assert(book.bestBid() == 0);
}


void testMarketOrderSweepsPriceLevels() {
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

    auto result =
        book.addMarketOrder(
            4,
            50,
            Side::Buy
        );

    assert(result.trades.size() == 3);

    assert(result.trades[0].price == 10001);
    assert(result.trades[1].price == 10002);
    assert(result.trades[2].price == 10003);

    assert(book.bestAsk() == 10003);
}


void testMarketOrderDoesNotRest() {
    OrderBook book;

    auto result =
        book.addMarketOrder(
            1,
            100,
            Side::Buy
        );

    assert(result.accepted());
    assert(result.trades.empty());

    assert(book.bestBid() == 0);
    assert(book.bestAsk() == 0);

    assert(!book.cancelOrder(1));
}


// =============================================
// IOC TESTS
// =============================================

void testIocBuyPartiallyFillsAndDoesNotRest() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10005,
        40,
        Side::Sell
    });

    auto result =
        book.addImmediateOrCancelOrder({
            2,
            10005,
            100,
            Side::Buy
        });

    assert(result.accepted());

    assert(result.trades.size() == 1);
    assert(result.trades[0].quantity == 40);

    assert(book.bestBid() == 0);
    assert(!book.cancelOrder(2));
}


void testIocRespectsLimitPrice() {
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

    auto result =
        book.addImmediateOrCancelOrder({
            4,
            10002,
            100,
            Side::Buy
        });

    assert(result.accepted());

    assert(result.trades.size() == 2);

    assert(result.trades[0].price == 10001);
    assert(result.trades[1].price == 10002);

    assert(book.bestAsk() == 10003);
    assert(book.bestBid() == 0);
}


void testIocWithNoMatchDoesNotRest() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10010,
        50,
        Side::Sell
    });

    auto result =
        book.addImmediateOrCancelOrder({
            2,
            10005,
            100,
            Side::Buy
        });

    assert(result.accepted());
    assert(result.trades.empty());

    assert(book.bestAsk() == 10010);
    assert(book.bestBid() == 0);

    assert(!book.cancelOrder(2));
}


// =============================================
// FOK TESTS
// =============================================

void testFokExecutesWhenFullyFillable() {
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
        30,
        Side::Sell
    });

    book.addLimitOrder({
        3,
        10003,
        50,
        Side::Sell
    });


    auto result =
        book.addFillOrKillOrder({
            4,
            10002,
            50,
            Side::Buy
        });


    assert(result.accepted());

    assert(result.trades.size() == 2);

    assert(result.trades[0].price == 10001);
    assert(result.trades[0].quantity == 20);

    assert(result.trades[1].price == 10002);
    assert(result.trades[1].quantity == 30);


    assert(book.bestAsk() == 10003);

    assert(!book.cancelOrder(4));
}


void testFokFailsWithoutChangingBook() {
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
        30,
        Side::Sell
    });

    book.addLimitOrder({
        3,
        10003,
        50,
        Side::Sell
    });


    // Only 50 units exist at 10002 or better.
    //
    // We require 70.
    auto result =
        book.addFillOrKillOrder({
            4,
            10002,
            70,
            Side::Buy
        });


    assert(
        result.status ==
        OrderStatus::InsufficientLiquidity
    );

    assert(!result.accepted());

    assert(result.trades.empty());


    // The book MUST be untouched.
    assert(book.bestAsk() == 10001);


    // Prove Order 1 still exists.
    assert(book.cancelOrder(1));


    // After cancelling Order 1,
    // 10002 should now be best ask.
    assert(book.bestAsk() == 10002);
}


void testFokDoesNotUsePricesOutsideLimit() {
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
        10050,
        1000,
        Side::Sell
    });


    // There is loads of total liquidity,
    // but only 40 units at 10002 or better.
    auto result =
        book.addFillOrKillOrder({
            4,
            10002,
            50,
            Side::Buy
        });


    assert(
        result.status ==
        OrderStatus::InsufficientLiquidity
    );

    assert(result.trades.empty());


    // Nothing was consumed.
    assert(book.bestAsk() == 10001);
}


void testFokSellFullyExecutes() {
    OrderBook book;

    book.addLimitOrder({
        1,
        10005,
        20,
        Side::Buy
    });

    book.addLimitOrder({
        2,
        10004,
        30,
        Side::Buy
    });

    book.addLimitOrder({
        3,
        10003,
        50,
        Side::Buy
    });


    auto result =
        book.addFillOrKillOrder({
            4,
            10004,
            50,
            Side::Sell
        });


    assert(result.accepted());

    assert(result.trades.size() == 2);

    assert(result.trades[0].price == 10005);
    assert(result.trades[0].quantity == 20);

    assert(result.trades[1].price == 10004);
    assert(result.trades[1].quantity == 30);

    assert(book.bestBid() == 10003);
}


int main() {

    testNonCrossingOrders();
    testExactFill();
    testPartialFill();
    testPriceTimePriority();
    testSweepMultiplePriceLevels();

    testCancelExistingOrder();
    testCancelNonexistentOrder();
    testDuplicateOrderIdRejected();

    testModifyNonexistentOrder();
    testModifyQuantityToZeroCancels();
    testQuantityDecreaseKeepsPriority();
    testQuantityIncreaseLosesPriority();
    testModifyCanGenerateTrades();

    testMarketBuyConsumesBestAsk();
    testMarketSellConsumesBestBid();
    testMarketOrderSweepsPriceLevels();
    testMarketOrderDoesNotRest();

    testIocBuyPartiallyFillsAndDoesNotRest();
    testIocRespectsLimitPrice();
    testIocWithNoMatchDoesNotRest();

    testFokExecutesWhenFullyFillable();
    testFokFailsWithoutChangingBook();
    testFokDoesNotUsePricesOutsideLimit();
    testFokSellFullyExecutes();


    std::cout
        << "All order book tests passed.\n";


    return 0;
}