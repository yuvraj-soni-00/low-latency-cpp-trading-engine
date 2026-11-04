#pragma once 
#include "types.hpp"
#include "order.hpp" 
#include <bits/stdc++.h>
namespace trading{
    class OrderBook{
        std::map<Price, std::list<Order>, std::greater<Price>> bids_;
        std::map<Price, std::list<Order>> asks_;
        bool is_valid_order(const Order& order);
        public:

        bool add_order(const Order& order);
        bool cancel_order(OrderId order_id);

        void display_order_book() const;

        std::optional<Price> best_bid() const;
        std::optional<Price> best_ask() const;

        void empty();

        std::size_t order_count() const;

        std::size_t best_bid_order_count() const;
        std::size_t best_ask_order_count() const;

        std::optional<Price> best_buy_quantity() const;
        std::optional<Price> best_ask_quantity() const;

    };
};