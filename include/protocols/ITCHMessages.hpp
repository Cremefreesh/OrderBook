#pragma once

#include <array>
#include <cstdint>
#include <variant>

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

struct OrderCancel {
    std::uint16_t stock_locate{};
    std::uint16_t tracking_number{};
    std::uint64_t timestamp_ns{};

    std::uint64_t order_reference{};
    std::uint32_t cancelled_shares{};
};

struct OrderDelete {
    std::uint16_t stock_locate{};
    std::uint16_t tracking_number{};
    std::uint64_t timestamp_ns{};
    std::uint64_t order_reference{};
};

struct OrderExecuted {
    std::uint16_t stock_locate{};
    std::uint16_t tracking_number{};
    std::uint64_t timestamp_ns{};

    std::uint64_t order_reference{};
    std::uint32_t executed_shares{};
    std::uint64_t match_number{};
};

struct OrderReplace {
    std::uint16_t stock_locate{};
    std::uint16_t tracking_number{};
    std::uint64_t timestamp_ns{};

    std::uint64_t original_order_reference{};
    std::uint64_t new_order_reference{};

    std::uint32_t shares{};
    std::uint32_t price{};
};

using Message = std::variant<
    AddOrder,
    OrderCancel,
    OrderDelete,
    OrderExecuted,
    OrderReplace
>;


}