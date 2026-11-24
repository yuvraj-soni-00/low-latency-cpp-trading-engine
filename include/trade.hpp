#pragma once
#include "order.hpp"

namespace trading{
    struct trade{
        OrderId buy_order_Id;
        OrderId ask_order_Id;
        Price price;
        Quantity quantity;
    };
}