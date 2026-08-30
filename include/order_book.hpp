#pragma once

#include <functional>
#include <list>
#include <map>
#include <unordered_map>

#include "order.hpp"
#include "order_result.hpp"
#include "price_level.hpp"


class OrderBook {
public:
    OrderResult addLimitOrder(Order order);

    bool cancelOrder(OrderId id);

    OrderResult modifyOrder(
        OrderId id,
        Price newPrice,
        Quantity newQuantity
    );

    Price bestBid() const;
    Price bestAsk() const;


private:
    struct OrderLocation {
        Side side;
        Price price;
        std::list<Order>::iterator orderIt;
    };


    std::map<
        Price,
        PriceLevel,
        std::greater<Price>
    > bids_;

    std::map<
        Price,
        PriceLevel
    > asks_;


    std::unordered_map<
        OrderId,
        OrderLocation
    > orderIndex_;
};