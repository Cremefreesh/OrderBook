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


OrderResult OrderBook::addLimitOrder(Order order) {

    OrderResult result {
        OrderStatus::Accepted,
        {}
    };


    // -----------------------------------------
    // Reject duplicate active OrderIds
    // -----------------------------------------

    if (orderIndex_.find(order.id) != orderIndex_.end()) {

        result.status =
            OrderStatus::DuplicateOrderId;

        return result;
    }


    // =========================================
    // BUY ORDER
    // =========================================

    if (order.side == Side::Buy) {

        while (
            order.quantity > 0 &&
            !asks_.empty()
        ) {

            auto bestAskIt =
                asks_.begin();

            Price bestAskPrice =
                bestAskIt->first;


            if (order.price < bestAskPrice) {
                break;
            }


            PriceLevel& level =
                bestAskIt->second;


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


                result.trades.push_back(trade);


                order.quantity -=
                    tradedQuantity;

                restingOrder.quantity -=
                    tradedQuantity;


                if (restingOrder.quantity == 0) {

                    orderIndex_.erase(
                        restingOrder.id
                    );

                    level.orders.pop_front();
                }
            }


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


            if (order.price > bestBidPrice) {
                break;
            }


            PriceLevel& level =
                bestBidIt->second;


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


                result.trades.push_back(trade);


                order.quantity -=
                    tradedQuantity;

                restingOrder.quantity -=
                    tradedQuantity;


                if (restingOrder.quantity == 0) {

                    orderIndex_.erase(
                        restingOrder.id
                    );

                    level.orders.pop_front();
                }
            }


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


OrderResult OrderBook::modifyOrder(
    OrderId id,
    Price newPrice,
    Quantity newQuantity
) {

    auto indexIt =
        orderIndex_.find(id);


    // -----------------------------------------
    // Order doesn't exist
    // -----------------------------------------

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


    // -----------------------------------------
    // Quantity zero = cancellation
    // -----------------------------------------

    if (newQuantity == 0) {

        cancelOrder(id);

        return {
            OrderStatus::Accepted,
            {}
        };
    }


    // -----------------------------------------
    // Same price + quantity decrease
    //
    // Keep priority.
    // -----------------------------------------

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


    // -----------------------------------------
    // Price change OR quantity increase
    //
    // Lose priority.
    // -----------------------------------------

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


    // This is important:
    //
    // addLimitOrder() might produce trades.
    //
    // We now RETURN those trades instead of
    // silently throwing them away.
    return addLimitOrder(replacement);
}