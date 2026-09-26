#pragma once

#include <array>
#include <cstdint>

namespace itch {

enum class Side : char {
    Buy  = 'B',
    Sell = 'S'
};

struct AddOrder {
    std::uint16_t stock_locate{};
    std::uint16_t tracking_number{};

    std::uint64_t timestamp_ns{};

    std::uint64_t order_reference{};
    Side side{};

    std::uint32_t shares{};

    std::array<char, 8> stock{};

    std::uint32_t price{};
};

}