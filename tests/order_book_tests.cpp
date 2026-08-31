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

    assert(book.cancelOrder(1));
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

    assert(book.bestAsk() == 10001);

    assert(book.cancelOrder(1));
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


// =============================================
// VALIDATION TESTS
// =============================================

void testLimitOrderRejectsZeroQuantity() {
    OrderBook book;


    auto result =
        book.addLimitOrder({
            1,
            10000,
            0,
            Side::Buy
        });


    assert(
        result.status ==
        OrderStatus::InvalidQuantity
    );

    assert(!result.accepted());
    assert(result.trades.empty());

    assert(book.bestBid() == 0);
    assert(book.bestAsk() == 0);
}


void testLimitOrderRejectsZeroPrice() {
    OrderBook book;


    auto result =
        book.addLimitOrder({
            1,
            0,
            100,
            Side::Buy
        });


    assert(
        result.status ==
        OrderStatus::InvalidPrice
    );

    assert(!result.accepted());

    assert(book.bestBid() == 0);
}


void testLimitOrderRejectsNegativePrice() {
    OrderBook book;


    auto result =
        book.addLimitOrder({
            1,
            -100,
            100,
            Side::Sell
        });


    assert(
        result.status ==
        OrderStatus::InvalidPrice
    );

    assert(!result.accepted());

    assert(book.bestAsk() == 0);
}


void testMarketOrderRejectsZeroQuantity() {
    OrderBook book;


    auto result =
        book.addMarketOrder(
            1,
            0,
            Side::Buy
        );


    assert(
        result.status ==
        OrderStatus::InvalidQuantity
    );

    assert(!result.accepted());

    assert(book.bestBid() == 0);
    assert(book.bestAsk() == 0);
}


void testIocRejectsInvalidPrice() {
    OrderBook book;


    auto result =
        book.addImmediateOrCancelOrder({
            1,
            0,
            100,
            Side::Buy
        });


    assert(
        result.status ==
        OrderStatus::InvalidPrice
    );

    assert(!result.accepted());
}


void testFokRejectsZeroQuantity() {
    OrderBook book;


    auto result =
        book.addFillOrKillOrder({
            1,
            10000,
            0,
            Side::Buy
        });


    assert(
        result.status ==
        OrderStatus::InvalidQuantity
    );

    assert(!result.accepted());
}


void testInvalidModifyPriceDoesNotChangeOrder() {
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
            0,
            50
        );


    assert(
        result.status ==
        OrderStatus::InvalidPrice
    );

    assert(!result.accepted());


    // Original order must still be intact.
    assert(book.bestBid() == 10000);


    // And it should still be cancellable.
    assert(book.cancelOrder(1));
}

void testTopBidsReturnsCorrectLevels() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10000,
        30,
        Side::Buy
    });

    book.addLimitOrder({
        2,
        10000,
        20,
        Side::Buy
    });

    book.addLimitOrder({
        3,
        9999,
        40,
        Side::Buy
    });

    book.addLimitOrder({
        4,
        9998,
        50,
        Side::Buy
    });


    auto bids =
        book.topBids(2);


    assert(bids.size() == 2);


    assert(bids[0].price == 10000);
    assert(bids[0].totalQuantity == 50);
    assert(bids[0].orderCount == 2);


    assert(bids[1].price == 9999);
    assert(bids[1].totalQuantity == 40);
    assert(bids[1].orderCount == 1);
}


void testTopAsksReturnsCorrectLevels() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10005,
        25,
        Side::Sell
    });

    book.addLimitOrder({
        2,
        10005,
        35,
        Side::Sell
    });

    book.addLimitOrder({
        3,
        10006,
        40,
        Side::Sell
    });

    book.addLimitOrder({
        4,
        10007,
        50,
        Side::Sell
    });


    auto asks =
        book.topAsks(2);


    assert(asks.size() == 2);


    assert(asks[0].price == 10005);
    assert(asks[0].totalQuantity == 60);
    assert(asks[0].orderCount == 2);


    assert(asks[1].price == 10006);
    assert(asks[1].totalQuantity == 40);
    assert(asks[1].orderCount == 1);
}


void testSnapshotDepthGreaterThanBook() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10000,
        50,
        Side::Buy
    });


    auto bids =
        book.topBids(10);


    assert(bids.size() == 1);

    assert(bids[0].price == 10000);
    assert(bids[0].totalQuantity == 50);
    assert(bids[0].orderCount == 1);
}


void testZeroSnapshotDepthReturnsEmpty() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10000,
        50,
        Side::Buy
    });


    auto bids =
        book.topBids(0);


    auto asks =
        book.topAsks(0);


    assert(bids.empty());
    assert(asks.empty());
}

void testLimitOrderReportsRemainingQuantity() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10005,
        40,
        Side::Sell
    });


    auto result =
        book.addLimitOrder({
            2,
            10005,
            100,
            Side::Buy
        });


    assert(result.accepted());

    assert(result.trades.size() == 1);

    assert(
        result.remainingQuantity ==
        60
    );


    // The remaining 60 should have rested.
    assert(book.bestBid() == 10005);
}


void testMarketOrderReportsUnfilledQuantity() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10005,
        30,
        Side::Sell
    });


    auto result =
        book.addMarketOrder(
            2,
            100,
            Side::Buy
        );


    assert(result.accepted());

    assert(
        result.remainingQuantity ==
        70
    );


    // Market remainder must not rest.
    assert(book.bestBid() == 0);
}


void testIocReportsCancelledRemainder() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10005,
        25,
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

    assert(
        result.remainingQuantity ==
        75
    );


    // IOC remainder disappears.
    assert(book.bestBid() == 0);
}


void testFullyFilledOrderReportsZeroRemaining() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10005,
        100,
        Side::Sell
    });


    auto result =
        book.addMarketOrder(
            2,
            100,
            Side::Buy
        );


    assert(result.accepted());

    assert(
        result.remainingQuantity ==
        0
    );
}


void testFailedFokReportsEntireQuantityRemaining() {
    OrderBook book;


    book.addLimitOrder({
        1,
        10005,
        20,
        Side::Sell
    });


    auto result =
        book.addFillOrKillOrder({
            2,
            10005,
            100,
            Side::Buy
        });


    assert(
        result.status ==
        OrderStatus::InsufficientLiquidity
    );


    assert(
        result.remainingQuantity ==
        100
    );


    assert(result.trades.empty());
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

    testLimitOrderRejectsZeroQuantity();
    testLimitOrderRejectsZeroPrice();
    testLimitOrderRejectsNegativePrice();
    testMarketOrderRejectsZeroQuantity();
    testIocRejectsInvalidPrice();
    testFokRejectsZeroQuantity();
    testInvalidModifyPriceDoesNotChangeOrder();

    testLimitOrderReportsRemainingQuantity();
    testMarketOrderReportsUnfilledQuantity();
    testIocReportsCancelledRemainder();
    testFullyFilledOrderReportsZeroRemaining();
    testFailedFokReportsEntireQuantityRemaining();


    std::cout
        << "All order book tests passed.\n";


    return 0;
}