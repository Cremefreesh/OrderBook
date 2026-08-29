#pragma once

#include "order.hpp"

struct Trade {
    OrderId buyOrderId;
    OrderId sellOrderId;
    Price price;
    Quantity quantity;
};