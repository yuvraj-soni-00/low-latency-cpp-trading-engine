#include "order_book.hpp"
namespace trading{
    bool OrderBook::is_valid_order(const Order& order){
        if (order.price<=0) return false;
        if (order.side!=Side::Buy && order.side!=Side::Sell) return false;
        if (order.price % TICK_SIZE != 0 || order.quantity==0) return false;
        if (order_index.find(order.orderid)!=order_index.end()) return false;
        return true;
    }

    bool OrderBook::add_order(const Order& order){
        if (!is_valid_order(order)) return false;
        if (order.side==Side::Buy){
            auto& level = bids_[order.price];
            level.push_back(order);
            auto iter = std::prev(level.end());
            order_index.emplace(
                order.orderid,
                Orderloc{order.price, order.side, iter}
            );
        }
        else{
            auto& level = asks_[order.price];
            level.push_back(order);
            auto iter = std::prev(level.end());
            order_index.emplace(
                order.orderid,
                Orderloc{order.price, order.side, iter}
            );
        }
        return true;
    }
}
    