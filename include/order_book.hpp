#pragma once
#include "order.hpp" 
#include <map>
#include <list>
#include <functional>
#include <unordered_map>
#include <optional>
#include <cstddef>

namespace trading{
    class OrderBook{
        struct Orderloc{
            Price price{};
            Side side{Side::Buy};
            std::list<Order>::iterator order_iter;
        };
        std::map<Price, std::list<Order>, std::greater<Price>> bids_;
        std::map<Price, std::list<Order>> asks_;
        std::unordered_map<OrderId, Orderloc> order_index;
        bool is_valid_order(const Order& order) const;
        public:

        bool add_order(const Order& order);
        bool cancel_order(OrderId order_id);

        void display_order_book() const;
        void display_detailed_order_book() const;

        std::optional<Price> best_bid() const;
        std::optional<Price> best_ask() const;
        std::optional<Price> spread() const;

        bool empty() const;

        std::size_t order_count() const;

        std::size_t best_bid_order_count() const;
        std::size_t best_ask_order_count() const;

        Quantity best_bid_quantity() const;
        Quantity best_ask_quantity() const;
    };
};