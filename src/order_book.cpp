#include "order_book.hpp"

#include <algorithm>
#include <cstdint>
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


std::vector<LevelSnapshot> OrderBook::topBids(
    std::size_t depth
) const {

    std::vector<LevelSnapshot> snapshots;

    snapshots.reserve(depth);


    std::size_t levelsAdded = 0;


    for (const auto& [price, level] : bids_) {

        if (levelsAdded >= depth) {
            break;
        }


        std::uint64_t totalQuantity = 0;


        for (const Order& order : level.orders) {

            totalQuantity +=
                order.quantity;
        }


        snapshots.push_back({
            price,
            totalQuantity,
            level.orders.size()
        });


        ++levelsAdded;
    }


    return snapshots;
}


std::vector<LevelSnapshot> OrderBook::topAsks(
    std::size_t depth
) const {

    std::vector<LevelSnapshot> snapshots;

    snapshots.reserve(depth);


    std::size_t levelsAdded = 0;


    for (const auto& [price, level] : asks_) {

        if (levelsAdded >= depth) {
            break;
        }


        std::uint64_t totalQuantity = 0;


        for (const Order& order : level.orders) {

            totalQuantity +=
                order.quantity;
        }


        snapshots.push_back({
            price,
            totalQuantity,
            level.orders.size()
        });


        ++levelsAdded;
    }


    return snapshots;
}


// =============================================
// CHECK WHETHER BUY CAN FULLY EXECUTE
// =============================================

bool OrderBook::canFullyFillBuy(
    Quantity quantity,
    Price limitPrice
) const {

    std::uint64_t availableQuantity = 0;


    for (const auto& [price, level] : asks_) {

        if (price > limitPrice) {
            break;
        }


        for (const Order& order : level.orders) {

            availableQuantity +=
                order.quantity;


            if (availableQuantity >= quantity) {
                return true;
            }
        }
    }


    return false;
}


// =============================================
// CHECK WHETHER SELL CAN FULLY EXECUTE
// =============================================

bool OrderBook::canFullyFillSell(
    Quantity quantity,
    Price limitPrice
) const {

    std::uint64_t availableQuantity = 0;


    for (const auto& [price, level] : bids_) {

        if (price < limitPrice) {
            break;
        }


        for (const Order& order : level.orders) {

            availableQuantity +=
                order.quantity;


            if (availableQuantity >= quantity) {
                return true;
            }
        }
    }


    return false;
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


        if (
            !isMarketOrder &&
            limitPrice < bestAskPrice
        ) {
            break;
        }


        PriceLevel& level =
            bestAskIt->second;


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


        if (
            !isMarketOrder &&
            limitPrice > bestBidPrice
        ) {
            break;
        }


        PriceLevel& level =
            bestBidIt->second;


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
}


// =============================================
// ADD LIMIT ORDER
// =============================================

OrderResult OrderBook::addLimitOrder(Order order) {

    OrderResult result {
        OrderStatus::Accepted,
        {},
        order.quantity
    };


    if (order.quantity == 0) {

        result.status =
            OrderStatus::InvalidQuantity;

        return result;
    }


    if (order.price <= 0) {

        result.status =
            OrderStatus::InvalidPrice;

        return result;
    }


    if (
        orderIndex_.find(order.id) !=
        orderIndex_.end()
    ) {

        result.status =
            OrderStatus::DuplicateOrderId;

        return result;
    }


    if (order.side == Side::Buy) {

        matchBuy(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );


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

    else {

        matchSell(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );


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


    result.remainingQuantity =
        order.quantity;


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
        {},
        quantity
    };


    if (quantity == 0) {

        result.status =
            OrderStatus::InvalidQuantity;

        return result;
    }


    if (
        orderIndex_.find(id) !=
        orderIndex_.end()
    ) {

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


    result.remainingQuantity =
        quantity;


    return result;
}


// =============================================
// ADD IMMEDIATE-OR-CANCEL ORDER
// =============================================

OrderResult OrderBook::addImmediateOrCancelOrder(
    Order order
) {

    OrderResult result {
        OrderStatus::Accepted,
        {},
        order.quantity
    };


    if (order.quantity == 0) {

        result.status =
            OrderStatus::InvalidQuantity;

        return result;
    }


    if (order.price <= 0) {

        result.status =
            OrderStatus::InvalidPrice;

        return result;
    }


    if (
        orderIndex_.find(order.id) !=
        orderIndex_.end()
    ) {

        result.status =
            OrderStatus::DuplicateOrderId;

        return result;
    }


    if (order.side == Side::Buy) {

        matchBuy(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );
    }

    else {

        matchSell(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );
    }


    result.remainingQuantity =
        order.quantity;


    return result;
}


// =============================================
// ADD FILL-OR-KILL ORDER
// =============================================

OrderResult OrderBook::addFillOrKillOrder(
    Order order
) {

    OrderResult result {
        OrderStatus::Accepted,
        {},
        order.quantity
    };


    if (order.quantity == 0) {

        result.status =
            OrderStatus::InvalidQuantity;

        return result;
    }


    if (order.price <= 0) {

        result.status =
            OrderStatus::InvalidPrice;

        return result;
    }


    if (
        orderIndex_.find(order.id) !=
        orderIndex_.end()
    ) {

        result.status =
            OrderStatus::DuplicateOrderId;

        return result;
    }


    bool canFullyFill = false;


    if (order.side == Side::Buy) {

        canFullyFill =
            canFullyFillBuy(
                order.quantity,
                order.price
            );
    }

    else {

        canFullyFill =
            canFullyFillSell(
                order.quantity,
                order.price
            );
    }


    if (!canFullyFill) {

        result.status =
            OrderStatus::InsufficientLiquidity;

        return result;
    }


    if (order.side == Side::Buy) {

        matchBuy(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );
    }

    else {

        matchSell(
            order.id,
            order.quantity,
            order.price,
            false,
            result
        );
    }


    result.remainingQuantity =
        order.quantity;


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
            {},
            newQuantity
        };
    }


    OrderLocation location =
        indexIt->second;


    Order& existingOrder =
        *location.orderIt;


    if (newQuantity == 0) {

        cancelOrder(id);

        return {
            OrderStatus::Accepted,
            {},
            0
        };
    }


    if (newPrice <= 0) {

        return {
            OrderStatus::InvalidPrice,
            {},
            newQuantity
        };
    }


    if (
        newPrice == existingOrder.price &&
        newQuantity <= existingOrder.quantity
    ) {

        existingOrder.quantity =
            newQuantity;

        return {
            OrderStatus::Accepted,
            {},
            newQuantity
        };
    }


    Side side =
        existingOrder.side;


    bool cancelled =
        cancelOrder(id);


    if (!cancelled) {

        return {
            OrderStatus::OrderNotFound,
            {},
            newQuantity
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