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

OrderDelete Parser::parse_order_delete(
    std::span<const std::uint8_t> bytes
) const {
    if (bytes.size() != 19) {
        throw std::runtime_error(
            "Invalid ITCH Order Delete message size"
        );
    }

    if (bytes[0] != 'D') {
        throw std::runtime_error(
            "Expected ITCH Order Delete message"
        );
    }

    OrderDelete message{};

    message.stock_locate =
        read_u16(bytes.data() + 1);

    message.tracking_number =
        read_u16(bytes.data() + 3);

    message.timestamp_ns =
        read_u48(bytes.data() + 5);

    message.order_reference =
        read_u64(bytes.data() + 11);

    return message;
}

OrderExecuted Parser::parse_order_executed(
    std::span<const std::uint8_t> bytes
) const {
    if (bytes.size() != 31) {
        throw std::runtime_error(
            "Invalid ITCH Order Executed message size"
        );
    }

    if (bytes[0] != 'E') {
        throw std::runtime_error(
            "Expected ITCH Order Executed message"
        );
    }

    OrderExecuted message{};

    message.stock_locate =
        read_u16(bytes.data() + 1);

    message.tracking_number =
        read_u16(bytes.data() + 3);

    message.timestamp_ns =
        read_u48(bytes.data() + 5);

    message.order_reference =
        read_u64(bytes.data() + 11);

    message.executed_shares =
        read_u32(bytes.data() + 19);

    message.match_number =
        read_u64(bytes.data() + 23);

    return message;
}

OrderReplace Parser::parse_order_replace(
    std::span<const std::uint8_t> bytes
) const {
    if (bytes.size() != 35) {
        throw std::runtime_error(
            "Invalid ITCH Order Replace message size"
        );
    }

    if (bytes[0] != 'U') {
        throw std::runtime_error(
            "Expected ITCH Order Replace message"
        );
    }

    OrderReplace message{};

    message.stock_locate =
        read_u16(bytes.data() + 1);

    message.tracking_number =
        read_u16(bytes.data() + 3);

    message.timestamp_ns =
        read_u48(bytes.data() + 5);

    message.original_order_reference =
        read_u64(bytes.data() + 11);

    message.new_order_reference =
        read_u64(bytes.data() + 19);

    message.shares =
        read_u32(bytes.data() + 27);

    message.price =
        read_u32(bytes.data() + 31);

    return message;
}

Message Parser::parse_message(
    std::span<const std::uint8_t> bytes
) const {
    if (bytes.empty()) {
        throw std::runtime_error("Empty ITCH message");
    }

    switch (bytes[0]) {
        case 'A':
            return parse_add_order(bytes);

        case 'X':
            return parse_order_cancel(bytes);

        case 'D':
            return parse_order_delete(bytes);

        case 'E':
            return parse_order_executed(bytes);

        case 'U':
            return parse_order_replace(bytes);

        default:
            throw std::runtime_error(
                "Unsupported ITCH message type"
            );
    }
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

order delete format
0       'D'
1-2     stock locate
3-4     tracking number
5-10    timestamp
11-18   order reference

order executed format
order #123
100 shares
    ↓

E says 40 shares executed
    ↓

60 shares remain


order replace format

*/


