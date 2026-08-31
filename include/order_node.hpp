#pragma once

#include <cstddef>

#include "order.hpp"


struct OrderNode {
    Order order;

    OrderNode* previous;
    OrderNode* next;

    OrderNode* nextFree;

    bool active;
};