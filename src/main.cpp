#include <cstdio>
#include <iostream>
#include <limits>
#include <string>
#include "order_book.hpp"

void wait_for_user() {
    std::cout << "\nPress Enter to continue...\n";
    // Clear any leftover characters/newlines in the stream buffer
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    // Wait for the actual Enter keypress
    std::cin.get();
}

int main() {
    std::cout << "=== Trading Engine Started ===" << std::endl;
    trading::OrderBook ob1;
    while (true) {
        std::printf("%-27s%-27s\n", "1.Add Order", "2.Cancel Order");
        std::printf("%-27s%-27s\n", "3.Best Bid", "4.Best ask");
        std::printf("%-27s%-27s\n", "5.Spread", "6.Display Order book");
        std::printf("%-27s%-27s\n", "7.Best Bid order count", "8.Best Ask order count");
        std::printf("%-27s%-27s\n", "9.Best Bid Order quantity", "10.Best Ask order quantity");
        std::printf("%-27s%-27s\n", "11.Order count", "12.Display detailed Order book");
        std::printf("13.Exit\n");

        int n;
        std::cin >> n;

        switch (n) {
            case 1: {
                trading::Order o1;
                std::string s;
                std::cout << "Enter Order id:";
                std::cin >> o1.orderid;
                std::cout << "Buy/Sell: ";
                std::cin >> s;
                std::cout << "Enter Quantity:";
                std::cin >> o1.quantity;
                std::cout << "Enter price (in paise):";
                std::cin >> o1.price;
                if (s == "Buy" || s == "buy") o1.side = trading::Side::Buy;
                else if (s == "sell" || s == "Sell") o1.side = trading::Side::Sell;
                else {
                    std::cout << "Invalid Side.";
                    continue;
                }
                if (ob1.add_order(o1)) std::cout << "Order Added successfully!" << std::endl;
                else std::cout << "Invalid Order." << std::endl;
                break;
            }
            case 2: {
                trading::OrderId oid;
                std::cout << "Enter Order Id: ";
                std::cin >> oid;
                ob1.cancel_order(oid) ? std::cout << "Order cancelled." << std::endl : std::cout << "Invalid Order" << std::endl;
                break;
            }
            case 3:
                if (const auto bid = ob1.best_bid())
                    std::cout << "Best Bid: " << *bid << std::endl;
                else
                    std::cout << "Best Bid: N/A" << std::endl;
                break;
            case 4:
                if (const auto ask = ob1.best_ask())
                    std::cout << "Best Ask: " << *ask << std::endl;
                else
                    std::cout << "Best Ask: N/A" << std::endl;
                break;
            case 5:
                if (const auto spread = ob1.spread())
                    std::cout << "Spread: " << *spread << std::endl;
                else
                    std::cout << "Spread: N/A" << std::endl;
                break;
            case 6:
                ob1.display_order_book();
                break;
            case 7:
                std::cout << "Best Bid order count: " << ob1.best_bid_order_count() << std::endl;
                break;
            case 8:
                std::cout << "Best Ask order count: " << ob1.best_ask_order_count() << std::endl;
                break;
            case 9:
                std::cout << "Best Bid quantity: " << ob1.best_bid_quantity() << std::endl;
                break;
            case 10:
                std::cout << "Best Ask quantity: " << ob1.best_ask_quantity() << std::endl;
                break;
            case 11:
                std::cout << "Total Orders: " << ob1.order_count() << std::endl;
                break;
            case 12:
                ob1.display_detailed_order_book();
                break;
            case 13:
                return 0;
            default:
                std::cout << "Invalid input." << std::endl;
                break;
        }
        wait_for_user();
    }
    return 0;
}