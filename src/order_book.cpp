#include "order_book.hpp"
#include <algorithm>

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

void OrderBook::addLimitOrder(Order order) {
    if (order.side == Side::Buy) {

        while (order.quantity > 0 && !asks_.empty()) {
            auto bestAskIt = asks_.begin();

            Price bestAskPrice = bestAskIt->first;

            if (order.price < bestAskPrice) {
                break;
            }

            PriceLevel& level = bestAskIt->second;

            while (order.quantity > 0 && !level.orders.empty()) {
                Order& restingOrder = level.orders.front();

                Quantity tradedQuantity =
                    std::min(order.quantity, restingOrder.quantity);

                order.quantity -= tradedQuantity;
                restingOrder.quantity -= tradedQuantity;

                if (restingOrder.quantity == 0) {
                    level.orders.pop_front();
                }
            }

            if (level.orders.empty()) {
                asks_.erase(bestAskIt);
            }
        }

        if (order.quantity > 0) {
            auto it = bids_.find(order.price);

            if (it == bids_.end()) {
                PriceLevel level {
                    order.price,
                    {}
                };

                level.orders.push_back(order);

                bids_.emplace(order.price, std::move(level));
            } else {
                it->second.orders.push_back(order);
            }
        }

    } else {

        while (order.quantity > 0 && !bids_.empty()) {
            auto bestBidIt = bids_.begin();

            Price bestBidPrice = bestBidIt->first;

            if (order.price > bestBidPrice) {
                break;
            }

            PriceLevel& level = bestBidIt->second;

            while (order.quantity > 0 && !level.orders.empty()) {
                Order& restingOrder = level.orders.front();

                Quantity tradedQuantity =
                    std::min(order.quantity, restingOrder.quantity);

                order.quantity -= tradedQuantity;
                restingOrder.quantity -= tradedQuantity;

                if (restingOrder.quantity == 0) {
                    level.orders.pop_front();
                }
            }

            if (level.orders.empty()) {
                bids_.erase(bestBidIt);
            }
        }

        if (order.quantity > 0) {
            auto it = asks_.find(order.price);

            if (it == asks_.end()) {
                PriceLevel level {
                    order.price,
                    {}
                };

                level.orders.push_back(order);

                asks_.emplace(order.price, std::move(level));
            } else {
                it->second.orders.push_back(order);
            }
        }
    }
}