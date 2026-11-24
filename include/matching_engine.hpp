#pragma once
#include "order_book.hpp"
#include "trade.hpp"

namespace trading{
    struct Orderprocessingresult{
        bool accepted{false};
        std::vector<trade> trades;
        Quantity remaining_quantity{0};
    };
    class Engine {
        public:
        Orderprocessingresult process_order(Order incoming_order);
        bool cancel_order(OrderId orderId);
        const OrderBook& order_book() const;
        const std::vector<trade>& trading_history() const;

        private:
        OrderBook order_book_;
        std::vector<trade> trade_history;
    };
}