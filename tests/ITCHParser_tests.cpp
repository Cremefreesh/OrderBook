#include "protocols/ITCHParser.hpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <iostream>

void test_add_order_parser() {
    std::array<std::uint8_t, 36> bytes{};

    // Message type
    bytes[0] = 'A';

    // Stock locate = 1
    bytes[1] = 0x00;
    bytes[2] = 0x01;

    // Tracking number = 2
    bytes[3] = 0x00;
    bytes[4] = 0x02;

    // Timestamp = 1000 ns
    bytes[5]  = 0x00;
    bytes[6]  = 0x00;
    bytes[7]  = 0x00;
    bytes[8]  = 0x00;
    bytes[9]  = 0x03;
    bytes[10] = 0xE8;

    // Order reference = 12345
    bytes[11] = 0x00;
    bytes[12] = 0x00;
    bytes[13] = 0x00;
    bytes[14] = 0x00;
    bytes[15] = 0x00;
    bytes[16] = 0x00;
    bytes[17] = 0x30;
    bytes[18] = 0x39;

    // Side
    bytes[19] = 'B';

    // Shares = 100
    bytes[20] = 0x00;
    bytes[21] = 0x00;
    bytes[22] = 0x00;
    bytes[23] = 0x64;

    // Stock = "AAPL    "
    bytes[24] = 'A';
    bytes[25] = 'A';
    bytes[26] = 'P';
    bytes[27] = 'L';
    bytes[28] = ' ';
    bytes[29] = ' ';
    bytes[30] = ' ';
    bytes[31] = ' ';

    // Price = 224.5000 -> integer representation 2,245,000
    bytes[32] = 0x00;
    bytes[33] = 0x22;
    bytes[34] = 0x41;
    bytes[35] = 0x88;

    itch::Parser parser;

    const auto message = parser.parse_add_order(bytes);

    assert(message.stock_locate == 1);
    assert(message.tracking_number == 2);
    assert(message.timestamp_ns == 1000);
    assert(message.order_reference == 12345);

    assert(message.side == itch::Side::Buy);
    assert(message.shares == 100);

    assert(message.stock[0] == 'A');
    assert(message.stock[1] == 'A');
    assert(message.stock[2] == 'P');
    assert(message.stock[3] == 'L');

    assert(message.price == 2245000);

    std::cout << "Add Order parser test passed!\n";
}

int main() {
    test_add_order_parser();

    std::cout << "All ITCH parser tests passed!\n";

    return 0;
}