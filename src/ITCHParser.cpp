#include "protocols/ITCHParser.hpp"

namespace itch {

std::uint16_t Parser::read_u16(const std::uint8_t* data) {
    return
        (static_cast<std::uint16_t>(data[0]) << 8) |
        (static_cast<std::uint16_t>(data[1]));
}

std::uint32_t Parser::read_u32(const std::uint8_t* data) {
    return
        (static_cast<std::uint32_t>(data[0]) << 24) |
        (static_cast<std::uint32_t>(data[1]) << 16) |
        (static_cast<std::uint32_t>(data[2]) << 8) |
        (static_cast<std::uint32_t>(data[3]));
}

std::uint64_t Parser::read_u48(const std::uint8_t* data) {
    return
        (static_cast<std::uint64_t>(data[0]) << 40) |
        (static_cast<std::uint64_t>(data[1]) << 32) |
        (static_cast<std::uint64_t>(data[2]) << 24) |
        (static_cast<std::uint64_t>(data[3]) << 16) |
        (static_cast<std::uint64_t>(data[4]) << 8) |
        (static_cast<std::uint64_t>(data[5]));
}

std::uint64_t Parser::read_u64(const std::uint8_t* data) {
    return
        (static_cast<std::uint64_t>(data[0]) << 56) |
        (static_cast<std::uint64_t>(data[1]) << 48) |
        (static_cast<std::uint64_t>(data[2]) << 40) |
        (static_cast<std::uint64_t>(data[3]) << 32) |
        (static_cast<std::uint64_t>(data[4]) << 24) |
        (static_cast<std::uint64_t>(data[5]) << 16) |
        (static_cast<std::uint64_t>(data[6]) << 8) |
        (static_cast<std::uint64_t>(data[7]));
}


AddOrder Parser::parse_add_order(
    std::span<const std::uint8_t> bytes
) const {
    AddOrder message{};

    return message;
}




}