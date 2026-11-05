#pragma once
#include "types.hpp"
#include <iterator>
namespace trading{
    struct Order{
        OrderId orderid{};
        Quantity quantity{};
        Price price{};
        Side side{Side::Buy};
    };
    struct Orderloc{
        Price price{};
        Side side{Side::Buy};
        std::list<Order>::iterator order_iter;
    };
};