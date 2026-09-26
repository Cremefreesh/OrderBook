#pragma once

#include <cstdint>

namespace itch {

enum class Side : std::uint8_t {
    Buy,
    Sell
};

struct AddOrder {
    std::uint64_t order_id;
    std::uint32_t quantity;
    std::uint32_t price;
    Side side;
};

struct CancelOrder {
    std::uint64_t order_id;
    std::uint32_t quantity;
};

}