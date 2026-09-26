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


/*
| Type | Message | Use Case |
|---|---|---|
| `A` | Add Order | New visible order enters book |
| `F` | Add Order with MPID Attribution | Same idea, but includes participant attribution |
| `E` | Order Executed | Some/all of an existing order trades |
| `C` | Order Executed With Price | Execution where trade price is explicitly supplied |
| `X` | Order Cancel | Reduce quantity of an existing order |
| `D` | Order Delete | Remove an order entirely |
| `U` | Order Replace | Replace order with new ID / quantity / price |
| `P` | Trade | Trade against a non-displayable order |
| `Q` | Cross Trade | Opening/closing/etc. cross transaction |
| `B` | Broken Trade | Previously reported trade is broken/cancelled |
*/
