#pragma once
#include <cstdint>

namespace trading{
    using OrderId = std::uint64_t;
    using Price = std::int64_t;
    using Quantity = std::uint64_t;

    enum class Side{
        Buy,
        Sell
    };

    constexpr Price TICK_SIZE = 5;
}



