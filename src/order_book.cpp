#include "order_book.hpp"

Price OrderBook::bestBid() const {
    if (bids_.empty()) {
        return 0;
    }

    return bids_.begin()->first;
}

Price OrderBook::bestAsk() const {
    if (asks_.empty()) {
        return 0;
    }

    return asks_.begin()->first;
}

