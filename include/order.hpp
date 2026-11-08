#pragma once
#include "types.hpp"
namespace trading{
    struct Order{
        OrderId orderid{};
        Quantity quantity{};
        Price price{};
        Side side{Side::Buy};
    };
    
};