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

    // -------------------------------------------------
    // Reject duplicate active OrderIds
    // -------------------------------------------------

    if (orderIndex_.find(order.id) != orderIndex_.end()) {
        return trades;
    }


    // =========================
    // BUY ORDER
    // =========================

    if (order.side == Side::Buy) {

        while (order.quantity > 0 && !asks_.empty()) {

            auto bestAskIt = asks_.begin();

            Price bestAskPrice = bestAskIt->first;

            // The cheapest seller is still too expensive.
            if (order.price < bestAskPrice) {
                break;
            }

            PriceLevel& level = bestAskIt->second;

            // Match orders at this price level in FIFO order.
            while (order.quantity > 0 && !level.orders.empty()) {

                Order& restingOrder = level.orders.front();

                Quantity tradedQuantity =
                    std::min(order.quantity, restingOrder.quantity);

                Trade trade {
                    order.id,
                    restingOrder.id,
                    restingOrder.price,
                    tradedQuantity
                };

                trades.push_back(trade);

                order.quantity -= tradedQuantity;
                restingOrder.quantity -= tradedQuantity;

                // Resting order has been completely filled.
                if (restingOrder.quantity == 0) {

                    orderIndex_.erase(restingOrder.id);

                    level.orders.pop_front();
                }
            }

            // No orders remain at this ask price.
            if (level.orders.empty()) {
                asks_.erase(bestAskIt);
            }
        }

        // If some of the incoming order remains,
        // place it onto the bid book.
        if (order.quantity > 0) {

            auto levelIt = bids_.find(order.price);

            // Price level does not exist yet.
            if (levelIt == bids_.end()) {

                PriceLevel level {
                    order.price,
                    {}
                };

                auto [newLevelIt, inserted] =
                    bids_.emplace(order.price, std::move(level));

                newLevelIt->second.orders.push_back(order);

                auto orderIt =
                    std::prev(newLevelIt->second.orders.end());

                orderIndex_[order.id] = {
                    Side::Buy,
                    order.price,
                    orderIt
                };

            } else {

                // Price level already exists.
                levelIt->second.orders.push_back(order);

                auto orderIt =
                    std::prev(levelIt->second.orders.end());

                orderIndex_[order.id] = {
                    Side::Buy,
                    order.price,
                    orderIt
                };
            }
        }
    }

    // =========================
    // SELL ORDER
    // =========================

    else {

        while (order.quantity > 0 && !bids_.empty()) {

            auto bestBidIt = bids_.begin();

            Price bestBidPrice = bestBidIt->first;

            // The highest buyer is offering too little.
            if (order.price > bestBidPrice) {
                break;
            }

            PriceLevel& level = bestBidIt->second;

            // Match orders at this price level in FIFO order.
            while (order.quantity > 0 && !level.orders.empty()) {

                Order& restingOrder = level.orders.front();

                Quantity tradedQuantity =
                    std::min(order.quantity, restingOrder.quantity);

                Trade trade {
                    restingOrder.id,
                    order.id,
                    restingOrder.price,
                    tradedQuantity
                };

                trades.push_back(trade);

                order.quantity -= tradedQuantity;
                restingOrder.quantity -= tradedQuantity;

                // Resting order has been completely filled.
                if (restingOrder.quantity == 0) {

                    orderIndex_.erase(restingOrder.id);

                    level.orders.pop_front();
                }
            }

            // No orders remain at this bid price.
            if (level.orders.empty()) {
                bids_.erase(bestBidIt);
            }
        }

        // If some of the incoming order remains,
        // place it onto the ask book.
        if (order.quantity > 0) {

            auto levelIt = asks_.find(order.price);

            // Price level does not exist yet.
            if (levelIt == asks_.end()) {

                PriceLevel level {
                    order.price,
                    {}
                };

                auto [newLevelIt, inserted] =
                    asks_.emplace(order.price, std::move(level));

                newLevelIt->second.orders.push_back(order);

                auto orderIt =
                    std::prev(newLevelIt->second.orders.end());

                orderIndex_[order.id] = {
                    Side::Sell,
                    order.price,
                    orderIt
                };

            } else {

                // Price level already exists.
                levelIt->second.orders.push_back(order);

                auto orderIt =
                    std::prev(levelIt->second.orders.end());

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

    // Step 1:
    // Find the order's location using our hash table.
    auto indexIt = orderIndex_.find(id);

    if (indexIt == orderIndex_.end()) {
        return false;
    }

    OrderLocation location = indexIt->second;

    // =========================
    // CANCEL BUY
    // =========================

    if (location.side == Side::Buy) {

        auto levelIt = bids_.find(location.price);

        if (levelIt == bids_.end()) {
            return false;
        }

        // Remove the exact order using its stored iterator.
        levelIt->second.orders.erase(location.orderIt);

        // If that was the final order at this price,
        // remove the entire price level.
        if (levelIt->second.orders.empty()) {
            bids_.erase(levelIt);
        }
    }

    // =========================
    // CANCEL SELL
    // =========================

    else {

        auto levelIt = asks_.find(location.price);

        if (levelIt == asks_.end()) {
            return false;
        }

        levelIt->second.orders.erase(location.orderIt);

        if (levelIt->second.orders.empty()) {
            asks_.erase(levelIt);
        }
    }

    // The order no longer exists, so remove it
    // from our lookup table too.
    orderIndex_.erase(indexIt);

    return true;
}