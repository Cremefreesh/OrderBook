#include "order_book.hpp"

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

void OrderBook::addLimitOrder(const Order& order) {
    if (order.side == Side::Buy) {
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
    } else {
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