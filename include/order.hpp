#pragma once

#include <cstdint>

using OrderId = std::uint64_t;
using Price = std::int64_t;
using Quantity = std::uint32_t;

enum class Side {
    Buy,
    Sell
};

struct Order {
    OrderId id;
    Price price;
    Quantity quantity;
    Side side;
};