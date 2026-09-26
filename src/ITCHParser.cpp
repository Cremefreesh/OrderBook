#include "protocols/ITCHParser.hpp"
#include <stdexcept>


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
     
    if (bytes.size() != 36) {
        throw std::runtime_error(
            "Invalid ITCH Add Order message size"
        );
    }

    if (bytes[0] != 'A') {
        throw std::runtime_error(
            "Expected ITCH Add Order message"
        );
    }

    AddOrder message{};

    message.stock_locate =
        read_u16(bytes.data() + 1);

    message.tracking_number =
        read_u16(bytes.data() + 3);

    message.timestamp_ns =
        read_u48(bytes.data() + 5);

    message.order_reference =
        read_u64(bytes.data() + 11);

    message.side =
        static_cast<Side>(bytes[19]);

    message.shares =
        read_u32(bytes.data() + 20);

    for (std::size_t i = 0; i < 8; ++i) {
        message.stock[i] =
            static_cast<char>(bytes[24 + i]);
    }

    message.price =
        read_u32(bytes.data() + 32);

    return message;
}


OrderCancel Parser::parse_order_cancel(
    std::span<const std::uint8_t> bytes
) const {
    if (bytes.size() != 23) {
        throw std::runtime_error(
            "Invalid ITCH Order Cancel message size"
        );
    }

    if (bytes[0] != 'X') {
        throw std::runtime_error(
            "Expected ITCH Order Cancel message"
        );
    }

    OrderCancel message{};

    message.stock_locate =
        read_u16(bytes.data() + 1);

    message.tracking_number =
        read_u16(bytes.data() + 3);

    message.timestamp_ns =
        read_u48(bytes.data() + 5);

    message.order_reference =
        read_u64(bytes.data() + 11);

    message.cancelled_shares =
        read_u32(bytes.data() + 19);

    return message;
}



}


/*
add order message format:
0      message type ('A')
1-2    stock locate
3-4    tracking number
5-10   timestamp
11-18  order reference
19     side
20-23  shares
24-31  stock
32-35  price

cancel order format 

0       type = 'X'
1-2     stock locate
3-4     tracking number
5-10    timestamp
11-18   order reference
19-22   cancelled shares

*/


