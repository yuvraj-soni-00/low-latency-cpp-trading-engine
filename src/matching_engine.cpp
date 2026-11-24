#include <iostream>
#include "matching_engine.hpp"
#include <math.h>

namespace trading{
    bool Engine::cancel_order(OrderId orderId){
        return order_book_.cancel_order(orderId);
    }

    const OrderBook& Engine::order_book() const{
        return order_book_;
    }

    Orderprocessingresult Engine::process_order(Order incoming_order){
        Orderprocessingresult result;
        if (!order_book_.is_valid_order(incoming_order)) {
            result.accepted=false;
            return result;
        }

        result.accepted=true;
        result.remaining_quantity=incoming_order.quantity;

        if (incoming_order.side == Side::Buy){
            auto f_order = order_book_.best_ask_order();
            while (f_order!=std::nullopt && f_order->price<=incoming_order.price){
                Quantity execution_quantity = std::min(result.remaining_quantity, f_order->quantity);
                if (order_book_.reduce_order_quant(f_order->orderid, execution_quantity)){
                    result.remaining_quantity -= execution_quantity;
                    trade t{incoming_order.orderid, f_order->orderid, f_order->price, execution_quantity};
                    result.trades.emplace_back(t);
                    trade_history.emplace_back(t);
                }
                if (result.remaining_quantity==0) return result;
                f_order = order_book_.best_ask_order();
            }

        }
        else{
             auto f_order = order_book_.best_bid_order();
            while (f_order!=std::nullopt && f_order->price>=incoming_order.price){
                Quantity execution_quantity = std::min(result.remaining_quantity, f_order->quantity);
                if (order_book_.reduce_order_quant(f_order->orderid, execution_quantity)){
                    result.remaining_quantity -= execution_quantity;
                    trade t{f_order->orderid, incoming_order.orderid, f_order->price, execution_quantity};
                    result.trades.emplace_back(t);
                    trade_history.emplace_back(t);
                }
                if (result.remaining_quantity==0) return result;
                f_order = order_book_.best_bid_order();
            }
        }
        incoming_order.quantity = result.remaining_quantity;
        order_book_.add_order(incoming_order);
        return result;
    }

    const std::vector<trade>& Engine::trading_history() const{
        return trade_history;
    }
}