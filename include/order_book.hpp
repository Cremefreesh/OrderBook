#pragma once

#include <map>
#include <vector>
#include <unordered_map>

#include "order.hpp"
#include "price_level.hpp"
#include "trade.hpp"

class OrderBook {
public:
    std::vector<Trade> addLimitOrder(Order order);

    bool cancelOrder(OrderId id);

    Price bestBid() const;
    Price bestAsk() const;

private:
    struct OrderLocation {
        Side side;
        Price price;
        std::list<Order>::iterator orderIt;
    };

    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel> asks_;

    std::unordered_map<OrderId, OrderLocation> orderIndex_;
};

