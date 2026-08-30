#pragma once

#include <vector>

#include "trade.hpp"


enum class OrderStatus {
    Accepted,
    DuplicateOrderId,
    OrderNotFound
};


struct OrderResult {
    OrderStatus status;
    std::vector<Trade> trades;

    bool accepted() const {
        return status == OrderStatus::Accepted;
    }
};