#pragma once

#include <map>

#include "order.hpp"
#include "price_level.hpp"

class OrderBook {
public:
    void addLimitOrder(const Order& order);

    Price bestBid() const;
    Price bestAsk() const;

private:
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel> asks_;
};