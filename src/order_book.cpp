#include "order_book.hpp"

#include <algorithm>
#include <iterator>
#include <utility>


Price OrderBook::bestBid() const {
    if (bids_.empty()) {
        return 0;
    }

    return bids_.begin()->first;
}


Price OrderBook::bestAsk() const {
    if (asks_.empty()) {
        return 0;
    }

    return asks_.begin()->first;
}


// =============================================
// MATCH INCOMING BUY
// =============================================

void OrderBook::matchBuy(
    OrderId incomingId,
    Quantity& remainingQuantity,
    Price limitPrice,
    bool isMarketOrder,
    OrderResult& result
) {

    while (
        remainingQuantity > 0 &&
        !asks_.empty()
    ) {

        auto bestAskIt =
            asks_.begin();


        Price bestAskPrice =
            bestAskIt->first;


        // Limit orders must respect their price.
        //
        // Market orders ignore this check.
        if (
            !isMarketOrder &&
            limitPrice < bestAskPrice
        ) {
            break;
        }


        PriceLevel& level =
            bestAskIt->second;


        // FIFO matching within this price level.
        while (
            remainingQuantity > 0 &&
            !level.orders.empty()
        ) {

            Order& restingOrder =
                level.orders.front();


            Quantity tradedQuantity =
                std::min(
                    remainingQuantity,
                    restingOrder.quantity
                );


            Trade trade {
                incomingId,
                restingOrder.id,
                restingOrder.price,
                tradedQuantity
            };


            result.trades.push_back(trade);


            remainingQuantity -=
                tradedQuantity;

            restingOrder.quantity -=
                tradedQuantity;


            // Resting order fully filled.
            if (restingOrder.quantity == 0) {

                orderIndex_.erase(
                    restingOrder.id
                );

                level.orders.pop_front();
            }
        }


        // Entire ask level consumed.
        if (level.orders.empty()) {
            asks_.erase(bestAskIt);
        }
    }
}


// =============================================
// MATCH INCOMING SELL
// =============================================

void OrderBook::matchSell(
    OrderId incomingId,
    Quantity& remainingQuantity,
    Price limitPrice,
    bool isMarketOrder,
    OrderResult& result
) {

    while (
        remainingQuantity > 0 &&
        !bids_.empty()
    ) {

        auto bestBidIt =
            bids_.begin();


        Price bestBidPrice =
            bestBidIt->first;


        // Limit orders must respect their price.
        //
        // Market orders ignore this check.
        if (
            !isMarketOrder &&
            limitPrice > bestBidPrice
        ) {
            break;
        }


        PriceLevel& level =
            bestBidIt->second;


        // FIFO matching within this price level.
        while (
            remainingQuantity > 0 &&
            !level.orders.empty()
        ) {

            Order& restingOrder =
                level.orders.front();


            Quantity tradedQuantity =
                std::min(
                    remainingQuantity,
                    restingOrder.quantity
                );


            Trade trade {
                restingOrder.id,
                incomingId,
                restingOrder.price,
                tradedQuantity
            };


            result.trades.push_back(trade);


            remainingQuantity -=
                tradedQuantity;

            restingOrder.quantity -=
                tradedQuantity;


            // Resting order fully filled.
            if (restingOrder.quantity == 0) {

                orderIndex_.erase(
                    restingOrder.id
                );

                level.orders.pop_front();
            }
        }


        // Entire bid level consumed.
        if (level.orders.empty()) {
            bids_.erase(bestBidIt);
        }
    }
}


// =============================================
// ADD LIMIT ORDER
// =============================================

OrderResult OrderBook::addLimitOrder(Order order) {

    OrderResult result {
        OrderStatus::Accepted,
        {}
    };


    // Reject duplicate active OrderIds.
    if (orderIndex_.find(order.id) != orderIndex_.end()) {

        result.status =
            OrderStatus::DuplicateOrderId;

        return result;
    }


    // =========================================
    // BUY LIMIT ORDER
    // =========================================

    if (order.side == Side::Buy) {

        matchBuy(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );


        // Rest any unfilled quantity.
        if (order.quantity > 0) {

            auto levelIt =
                bids_.find(order.price);


            if (levelIt == bids_.end()) {

                PriceLevel level {
                    order.price,
                    {}
                };


                auto [newLevelIt, inserted] =
                    bids_.emplace(
                        order.price,
                        std::move(level)
                    );


                newLevelIt
                    ->second
                    .orders
                    .push_back(order);


                auto orderIt =
                    std::prev(
                        newLevelIt
                            ->second
                            .orders
                            .end()
                    );


                orderIndex_[order.id] = {
                    Side::Buy,
                    order.price,
                    orderIt
                };
            }

            else {

                levelIt
                    ->second
                    .orders
                    .push_back(order);


                auto orderIt =
                    std::prev(
                        levelIt
                            ->second
                            .orders
                            .end()
                    );


                orderIndex_[order.id] = {
                    Side::Buy,
                    order.price,
                    orderIt
                };
            }
        }
    }


    // =========================================
    // SELL LIMIT ORDER
    // =========================================

    else {

        matchSell(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );


        // Rest any unfilled quantity.
        if (order.quantity > 0) {

            auto levelIt =
                asks_.find(order.price);


            if (levelIt == asks_.end()) {

                PriceLevel level {
                    order.price,
                    {}
                };


                auto [newLevelIt, inserted] =
                    asks_.emplace(
                        order.price,
                        std::move(level)
                    );


                newLevelIt
                    ->second
                    .orders
                    .push_back(order);


                auto orderIt =
                    std::prev(
                        newLevelIt
                            ->second
                            .orders
                            .end()
                    );


                orderIndex_[order.id] = {
                    Side::Sell,
                    order.price,
                    orderIt
                };
            }

            else {

                levelIt
                    ->second
                    .orders
                    .push_back(order);


                auto orderIt =
                    std::prev(
                        levelIt
                            ->second
                            .orders
                            .end()
                    );


                orderIndex_[order.id] = {
                    Side::Sell,
                    order.price,
                    orderIt
                };
            }
        }
    }


    return result;
}


// =============================================
// ADD MARKET ORDER
// =============================================

OrderResult OrderBook::addMarketOrder(
    OrderId id,
    Quantity quantity,
    Side side
) {

    OrderResult result {
        OrderStatus::Accepted,
        {}
    };


    // Reject duplicate active OrderIds.
    if (orderIndex_.find(id) != orderIndex_.end()) {

        result.status =
            OrderStatus::DuplicateOrderId;

        return result;
    }


    if (side == Side::Buy) {

        matchBuy(
            id,
            quantity,
            0,
            true,
            result
        );
    }

    else {

        matchSell(
            id,
            quantity,
            0,
            true,
            result
        );
    }


    // Market orders never rest.
    //
    // Any remaining quantity disappears.
    return result;
}


// =============================================
// CANCEL ORDER
// =============================================

bool OrderBook::cancelOrder(OrderId id) {

    auto indexIt =
        orderIndex_.find(id);


    if (indexIt == orderIndex_.end()) {
        return false;
    }


    OrderLocation location =
        indexIt->second;


    // =========================================
    // CANCEL BUY
    // =========================================

    if (location.side == Side::Buy) {

        auto levelIt =
            bids_.find(location.price);


        if (levelIt == bids_.end()) {
            return false;
        }


        levelIt
            ->second
            .orders
            .erase(location.orderIt);


        if (
            levelIt
                ->second
                .orders
                .empty()
        ) {

            bids_.erase(levelIt);
        }
    }


    // =========================================
    // CANCEL SELL
    // =========================================

    else {

        auto levelIt =
            asks_.find(location.price);


        if (levelIt == asks_.end()) {
            return false;
        }


        levelIt
            ->second
            .orders
            .erase(location.orderIt);


        if (
            levelIt
                ->second
                .orders
                .empty()
        ) {

            asks_.erase(levelIt);
        }
    }


    orderIndex_.erase(indexIt);


    return true;
}


// =============================================
// MODIFY ORDER
// =============================================

OrderResult OrderBook::modifyOrder(
    OrderId id,
    Price newPrice,
    Quantity newQuantity
) {

    auto indexIt =
        orderIndex_.find(id);


    if (indexIt == orderIndex_.end()) {

        return {
            OrderStatus::OrderNotFound,
            {}
        };
    }


    OrderLocation location =
        indexIt->second;


    Order& existingOrder =
        *location.orderIt;


    // Quantity zero = cancellation.
    if (newQuantity == 0) {

        cancelOrder(id);

        return {
            OrderStatus::Accepted,
            {}
        };
    }


    // Same price + lower/equal quantity:
    //
    // keep queue priority.
    if (
        newPrice == existingOrder.price &&
        newQuantity <= existingOrder.quantity
    ) {

        existingOrder.quantity =
            newQuantity;

        return {
            OrderStatus::Accepted,
            {}
        };
    }


    // Price change or quantity increase:
    //
    // lose priority.

    Side side =
        existingOrder.side;


    bool cancelled =
        cancelOrder(id);


    if (!cancelled) {

        return {
            OrderStatus::OrderNotFound,
            {}
        };
    }


    Order replacement {
        id,
        newPrice,
        newQuantity,
        side
    };


    return addLimitOrder(replacement);
}