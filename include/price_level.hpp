#pragma once

#include <list>

#include "order.hpp"

struct PriceLevel {
    Price price;
    std::list<Order> orders;
};