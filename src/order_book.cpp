#include "order_book.hpp"
#include <cinttypes>
#include <cstdio>
#include <iterator>
namespace trading{
    bool OrderBook::is_valid_order(const Order& order) const{
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

    bool OrderBook::cancel_order(OrderId order_id){
        auto order = order_index.find(order_id);
        if (order==order_index.end()) return false;
        if(order->second.side==Side::Buy){
            auto& level = bids_[order->second.price];
            level.erase(order->second.order_iter);
            if (level.empty()) bids_.erase(order->second.price);
        }
        else{
            auto& level = asks_[order->second.price];
            level.erase(order->second.order_iter);
            if (level.empty()) asks_.erase(order->second.price);
        }
        order_index.erase(order);
        return true;
    }

    void OrderBook::display_order_book() const{
        auto bid_it = bids_.begin();
        auto ask_it = asks_.begin();
        std::printf("%-45s %-45s\n\n","Buy","Sell");
        std::printf("%-15s%-15s%15s ","Orders","Quantity","Price");
        std::printf("%-15s%15s%15s\n\n","Price","Quantity","Orders");
        while(bid_it!=bids_.end() || ask_it!=asks_.end()){
            if(bid_it!=bids_.end()){
                size_t orders=bid_it->second.size();
                Quantity quant=0;
                for (const auto& it : bid_it->second){
                    quant+=it.quantity;
                }
                std::printf("%-15zu%-15" PRIu64 "%15" PRId64 " ",orders,quant,bid_it->first);
                ++bid_it;
            }
            else std::printf("%45s ","");
            if (ask_it!=asks_.end()){
                size_t orders=ask_it->second.size();
                Quantity quant=0;
                for (const auto& it : ask_it->second){
                    quant+=it.quantity;
                }
                std::printf("%-15" PRId64 "%15" PRIu64 "%15zu\n",ask_it->first,quant,orders);
                ++ask_it;
            }
            else std::printf("%45s\n","");
        }
    }
    void OrderBook::display_detailed_order_book() const{
        std::printf("Buy Orders\n\n");
        for (const auto& it : bids_){
            std::printf("Price: %" PRId64 "\n",it.first);
            for (auto& it1 : it.second){
                std::printf("%7s ID: %" PRIu64 " | Quantity: %" PRIu64 "\n","",it1.orderid,it1.quantity);
            }
        }
        std::printf("\n\nSell Orders\n\n");
        for (const auto& it : asks_){
            std::printf("Price: %" PRId64 "\n",it.first);
            for (const auto& it1 : it.second){
                std::printf("%7s ID: %" PRIu64 " | Quantity: %" PRIu64 "\n","",it1.orderid,it1.quantity);
            }
        }
    }

    std::optional<Price> OrderBook::best_ask() const{
        if (asks_.empty()) return std::nullopt;
        return asks_.begin()->first;
    }
    std::optional<Price> OrderBook::best_bid() const{
        if (bids_.empty()) return std::nullopt;
        return bids_.begin()->first;
    }
    std::optional<Price> OrderBook::spread() const{
        if (asks_.empty() || bids_.empty()) return std::nullopt;
        return asks_.begin()->first - bids_.begin()->first;
    }

    std::size_t OrderBook::best_bid_order_count() const{
        if (bids_.empty()) return 0;
        return bids_.begin()->second.size();
    }
    std::size_t OrderBook::best_ask_order_count() const{
        if (asks_.empty()) return 0;
        return asks_.begin()->second.size();
    }

    Quantity OrderBook::best_bid_quantity() const{
        Quantity bb_quant=0;
        if (bids_.empty()) return 0;
        for (auto& it : bids_.begin()->second){
            bb_quant += it.quantity;
        }
        return bb_quant;
    }
    Quantity OrderBook::best_ask_quantity() const{
        Quantity ba_quant=0;
        if (asks_.empty()) return 0;
        for (auto& it : asks_.begin()->second){
            ba_quant += it.quantity;
        }
        return ba_quant;
    }

    std::size_t OrderBook::order_count() const{
        return order_index.size();
    }

    bool OrderBook::empty() const{
        return order_index.empty();
    }

}
    