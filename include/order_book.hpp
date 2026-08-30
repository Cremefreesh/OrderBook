#pragma once

#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

#include "order.hpp"
#include "order_result.hpp"
#include "price_level.hpp"


struct LevelSnapshot {
    Price price;
    std::uint64_t totalQuantity;
    std::size_t orderCount;
};


class OrderBook {
public:
    OrderResult addLimitOrder(Order order);

    OrderResult addMarketOrder(
        OrderId id,
        Quantity quantity,
        Side side
    );

    OrderResult addImmediateOrCancelOrder(
        Order order
    );

    OrderResult addFillOrKillOrder(
        Order order
    );

    bool cancelOrder(OrderId id);

    OrderResult modifyOrder(
        OrderId id,
        Price newPrice,
        Quantity newQuantity
    );

    Price bestBid() const;
    Price bestAsk() const;

    std::vector<LevelSnapshot> topBids(
        std::size_t depth
    ) const;

    std::vector<LevelSnapshot> topAsks(
        std::size_t depth
    ) const;


private:
    struct OrderLocation {
        Side side;
        Price price;
        std::list<Order>::iterator orderIt;
    };


    void matchBuy(
        OrderId incomingId,
        Quantity& remainingQuantity,
        Price limitPrice,
        bool isMarketOrder,
        OrderResult& result
    );


    void matchSell(
        OrderId incomingId,
        Quantity& remainingQuantity,
        Price limitPrice,
        bool isMarketOrder,
        OrderResult& result
    );


    bool canFullyFillBuy(
        Quantity quantity,
        Price limitPrice
    ) const;


    bool canFullyFillSell(
        Quantity quantity,
        Price limitPrice
    ) const;


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