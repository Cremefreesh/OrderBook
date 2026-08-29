#pragma once

#include <map>
#include <vector>

#include "order.hpp"
#include "price_level.hpp"
#include "trade.hpp"

class OrderBook {
public:
    std::vector<Trade> addLimitOrder(Order order);

    Price bestBid() const;
    Price bestAsk() const;

private:
    std::map<Price, PriceLevel, std::greater<Price>> bids_;
    std::map<Price, PriceLevel> asks_;
};