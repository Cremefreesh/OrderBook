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


std::vector<Trade> OrderBook::addLimitOrder(Order order) {
    std::vector<Trade> trades;


    // -----------------------------------------
    // Reject duplicate active OrderIds
    // -----------------------------------------

    if (orderIndex_.find(order.id) != orderIndex_.end()) {
        return trades;
    }


    // =========================================
    // BUY ORDER
    // =========================================

    if (order.side == Side::Buy) {

        while (
            order.quantity > 0 &&
            !asks_.empty()
        ) {

            auto bestAskIt = asks_.begin();

            Price bestAskPrice =
                bestAskIt->first;


            // No price overlap.
            if (order.price < bestAskPrice) {
                break;
            }


            PriceLevel& level =
                bestAskIt->second;


            // Match FIFO within this price level.
            while (
                order.quantity > 0 &&
                !level.orders.empty()
            ) {

                Order& restingOrder =
                    level.orders.front();


                Quantity tradedQuantity =
                    std::min(
                        order.quantity,
                        restingOrder.quantity
                    );


                Trade trade {
                    order.id,
                    restingOrder.id,
                    restingOrder.price,
                    tradedQuantity
                };


                trades.push_back(trade);


                order.quantity -= tradedQuantity;
                restingOrder.quantity -= tradedQuantity;


                // Resting order completely filled.
                if (restingOrder.quantity == 0) {

                    orderIndex_.erase(
                        restingOrder.id
                    );

                    level.orders.pop_front();
                }
            }


            // Remove empty price level.
            if (level.orders.empty()) {
                asks_.erase(bestAskIt);
            }
        }


        // -----------------------------------------
        // Rest remaining BUY quantity
        // -----------------------------------------

        if (order.quantity > 0) {

            auto levelIt =
                bids_.find(order.price);


            // Price level doesn't exist.
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

            // Price level already exists.
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
    // SELL ORDER
    // =========================================

    else {

        while (
            order.quantity > 0 &&
            !bids_.empty()
        ) {

            auto bestBidIt =
                bids_.begin();


            Price bestBidPrice =
                bestBidIt->first;


            // No price overlap.
            if (order.price > bestBidPrice) {
                break;
            }


            PriceLevel& level =
                bestBidIt->second;


            // Match FIFO within this price level.
            while (
                order.quantity > 0 &&
                !level.orders.empty()
            ) {

                Order& restingOrder =
                    level.orders.front();


                Quantity tradedQuantity =
                    std::min(
                        order.quantity,
                        restingOrder.quantity
                    );


                Trade trade {
                    restingOrder.id,
                    order.id,
                    restingOrder.price,
                    tradedQuantity
                };


                trades.push_back(trade);


                order.quantity -= tradedQuantity;
                restingOrder.quantity -= tradedQuantity;


                // Resting order completely filled.
                if (restingOrder.quantity == 0) {

                    orderIndex_.erase(
                        restingOrder.id
                    );

                    level.orders.pop_front();
                }
            }


            // Remove empty price level.
            if (level.orders.empty()) {
                bids_.erase(bestBidIt);
            }
        }


        // -----------------------------------------
        // Rest remaining SELL quantity
        // -----------------------------------------

        if (order.quantity > 0) {

            auto levelIt =
                asks_.find(order.price);


            // Price level doesn't exist.
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

            // Price level already exists.
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


    return trades;
}


bool OrderBook::cancelOrder(OrderId id) {

    auto indexIt =
        orderIndex_.find(id);


    // Order doesn't exist.
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


        // Remove price level if now empty.
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


        // Remove price level if now empty.
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


bool OrderBook::modifyOrder(
    OrderId id,
    Price newPrice,
    Quantity newQuantity
) {

    // -----------------------------------------
    // Find existing order
    // -----------------------------------------

    auto indexIt =
        orderIndex_.find(id);


    if (indexIt == orderIndex_.end()) {
        return false;
    }


    OrderLocation location =
        indexIt->second;


    Order& existingOrder =
        *location.orderIt;


    // -----------------------------------------
    // Quantity = 0 behaves like cancellation
    // -----------------------------------------

    if (newQuantity == 0) {

        return cancelOrder(id);
    }


    // -----------------------------------------
    // SAME PRICE + QUANTITY DECREASE
    //
    // Keep queue priority.
    // -----------------------------------------

    if (
        newPrice == existingOrder.price &&
        newQuantity <= existingOrder.quantity
    ) {

        existingOrder.quantity =
            newQuantity;

        return true;
    }


    // -----------------------------------------
    // Otherwise priority is lost.
    //
    // This includes:
    //
    // 1. price change
    // 2. quantity increase
    // -----------------------------------------


    Side side =
        existingOrder.side;


    // Remove old order from the book.
    bool cancelled =
        cancelOrder(id);


    if (!cancelled) {
        return false;
    }


    // Create replacement order.
    Order replacement {
        id,
        newPrice,
        newQuantity,
        side
    };


    // Reinsert it.
    //
    // Because push_back is used when an order
    // rests, it goes to the back of the FIFO
    // queue and therefore loses time priority.
    addLimitOrder(replacement);


    return true;
}