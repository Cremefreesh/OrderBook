#pragma once

#include "protocols/ITCHMessages.hpp"

#include <cstddef>
#include <cstdint>
#include <span>

namespace itch {

class Parser {
public:
    AddOrder parse_add_order(
        std::span<const std::uint8_t> bytes
    ) const;

private:
    static std::uint16_t read_u16(
        const std::uint8_t* data
    );

    static std::uint32_t read_u32(
        const std::uint8_t* data
    );

    static std::uint64_t read_u48(
        const std::uint8_t* data
    );

    static std::uint64_t read_u64(
        const std::uint8_t* data
    );
};

}