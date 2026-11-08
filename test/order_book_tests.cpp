
#include "order_book.hpp"

#include <cassert>
#include <iostream>
#include <string>
#include <sstream>

using namespace trading;

// Helper function to create an Order
Order make_order(OrderId id, Side side, Price price, Quantity quantity) {

    Order order{};

    order.orderid = id;
    order.side = side;
    order.price = price;
    order.quantity = quantity;

    return order;
}

// Captures detailed order book output
std::string capture_detailed_display(const OrderBook& book) {

    std::ostringstream output;

    std::streambuf* original_buffer = std::cout.rdbuf();

    std::cout.rdbuf(output.rdbuf());

    book.display_detailed_order_book();

    std::cout.rdbuf(original_buffer);

    return output.str();
}


// --------------------------------------------------
// TEST 1: Empty Order Book
// --------------------------------------------------

void test_empty_order_book() {

    OrderBook book;

    assert(book.empty());
    assert(book.order_count() == 0);

    assert(!book.best_bid().has_value());
    assert(!book.best_ask().has_value());
    assert(!book.spread().has_value());

    assert(book.best_bid_quantity() == 0);
    assert(book.best_ask_quantity() == 0);

    assert(book.best_bid_order_count() == 0);
    assert(book.best_ask_order_count() == 0);

    std::cout << "[PASS] Empty order book\n";
}


// --------------------------------------------------
// TEST 2: Add Buy and Sell Orders
// --------------------------------------------------

void test_add_orders() {

    OrderBook book;

    assert(book.add_order(make_order(101, Side::Buy, 100, 10)));
    assert(book.add_order(make_order(102, Side::Buy, 105, 20)));

    assert(book.add_order(make_order(103, Side::Sell, 110, 15)));
    assert(book.add_order(make_order(104, Side::Sell, 115, 25)));

    assert(!book.empty());
    assert(book.order_count() == 4);

    std::cout << "[PASS] Add Buy and Sell orders\n";
}


// --------------------------------------------------
// TEST 3: Price Priority
// --------------------------------------------------

void test_price_priority() {

    OrderBook book;

    // Buy orders inserted in unsorted order
    assert(book.add_order(make_order(201, Side::Buy, 95, 10)));
    assert(book.add_order(make_order(202, Side::Buy, 110, 20)));
    assert(book.add_order(make_order(203, Side::Buy, 100, 15)));
    assert(book.add_order(make_order(204, Side::Buy, 105, 25)));

    // Sell orders inserted in unsorted order
    assert(book.add_order(make_order(301, Side::Sell, 125, 10)));
    assert(book.add_order(make_order(302, Side::Sell, 115, 20)));
    assert(book.add_order(make_order(303, Side::Sell, 120, 15)));
    assert(book.add_order(make_order(304, Side::Sell, 130, 25)));

    assert(book.best_bid().has_value());
    assert(book.best_ask().has_value());

    assert(*book.best_bid() == 110);
    assert(*book.best_ask() == 115);

    std::cout << "[PASS] Price priority\n";
}


// --------------------------------------------------
// TEST 4: FIFO Priority
// --------------------------------------------------

// void test_fifo_priority() {

//     OrderBook book;

//     // Three Buy orders at the same price
//     assert(book.add_order(make_order(401, Side::Buy, 100, 10)));
//     assert(book.add_order(make_order(402, Side::Buy, 100, 20)));
//     assert(book.add_order(make_order(403, Side::Buy, 100, 30)));

//     std::string output = capture_detailed_display(book);

//     auto first = output.find("401");
//     auto second = output.find("402");
//     auto third = output.find("403");

//     assert(first != std::string::npos);
//     assert(second != std::string::npos);
//     assert(third != std::string::npos);

//     assert(first < second);
//     assert(second < third);

//     std::cout << "[PASS] FIFO priority\n";
// }


// --------------------------------------------------
// TEST 5: Duplicate Order ID
// --------------------------------------------------

void test_duplicate_order_id() {

    OrderBook book;

    assert(book.add_order(make_order(501, Side::Buy, 100, 10)));

    // Duplicate on the same side
    assert(!book.add_order(make_order(501, Side::Buy, 105, 20)));

    // Duplicate on the opposite side
    assert(!book.add_order(make_order(501, Side::Sell, 110, 15)));

    assert(book.order_count() == 1);

    std::cout << "[PASS] Duplicate order ID\n";
}


// --------------------------------------------------
// TEST 6: Invalid Orders
// --------------------------------------------------

void test_invalid_orders() {

    OrderBook book;

    // Negative price
    assert(!book.add_order(make_order(601, Side::Buy, -100, 10)));

    // Zero price
    assert(!book.add_order(make_order(602, Side::Buy, 0, 10)));

    // Invalid tick size
    assert(!book.add_order(make_order(603, Side::Buy, 102, 10)));

    // Zero quantity
    assert(!book.add_order(make_order(604, Side::Buy, 100, 0)));

    // Invalid side
    assert(!book.add_order(
        make_order(605, static_cast<Side>(99), 100, 10)
    ));

    assert(book.empty());
    assert(book.order_count() == 0);

    std::cout << "[PASS] Invalid orders\n";
}


// --------------------------------------------------
// TEST 7: Cancel Buy and Sell Orders
// --------------------------------------------------

void test_cancel_orders() {

    OrderBook book;

    assert(book.add_order(make_order(701, Side::Buy, 100, 10)));
    assert(book.add_order(make_order(702, Side::Buy, 105, 20)));
    assert(book.add_order(make_order(703, Side::Sell, 110, 15)));

    assert(book.order_count() == 3);

    // Cancel Buy order
    assert(book.cancel_order(701));

    assert(book.order_count() == 2);
    assert(*book.best_bid() == 105);

    // Cancel Sell order
    assert(book.cancel_order(703));

    assert(book.order_count() == 1);
    assert(!book.best_ask().has_value());

    std::cout << "[PASS] Cancel Buy and Sell orders\n";
}


// --------------------------------------------------
// TEST 8: Cancel Nonexistent Order
// --------------------------------------------------

void test_cancel_nonexistent_order() {

    OrderBook book;

    assert(book.add_order(make_order(801, Side::Buy, 100, 10)));

    assert(!book.cancel_order(999));

    assert(book.order_count() == 1);

    // Cancelling the same order twice
    assert(book.cancel_order(801));
    assert(!book.cancel_order(801));

    assert(book.empty());

    std::cout << "[PASS] Cancel nonexistent order\n";
}


// --------------------------------------------------
// TEST 9: Remove Empty Price Level
// --------------------------------------------------

void test_remove_empty_price_level() {

    OrderBook book;

    assert(book.add_order(make_order(901, Side::Buy, 105, 10)));
    assert(book.add_order(make_order(902, Side::Buy, 100, 20)));

    assert(*book.best_bid() == 105);

    // Remove the only order at price 105
    assert(book.cancel_order(901));

    // Best bid should now become 100
    assert(*book.best_bid() == 100);

    assert(book.order_count() == 1);

    std::cout << "[PASS] Remove empty price level\n";
}


// --------------------------------------------------
// TEST 10: Market Statistics
// --------------------------------------------------

void test_market_statistics() {

    OrderBook book;

    assert(book.add_order(make_order(1001, Side::Buy, 105, 10)));
    assert(book.add_order(make_order(1002, Side::Buy, 105, 20)));
    assert(book.add_order(make_order(1003, Side::Buy, 100, 30)));

    assert(book.add_order(make_order(1004, Side::Sell, 110, 15)));
    assert(book.add_order(make_order(1005, Side::Sell, 110, 25)));
    assert(book.add_order(make_order(1006, Side::Sell, 115, 10)));

    // Best prices
    assert(*book.best_bid() == 105);
    assert(*book.best_ask() == 110);

    // Spread
    assert(*book.spread() == 5);

    // Total quantities at best price levels
    assert(book.best_bid_quantity() == 30);
    assert(book.best_ask_quantity() == 40);

    // Number of orders at best price levels
    assert(book.best_bid_order_count() == 2);
    assert(book.best_ask_order_count() == 2);

    // Total active orders
    assert(book.order_count() == 6);

    std::cout << "[PASS] Market statistics\n";
}


// --------------------------------------------------
// TEST 11: Spread Edge Cases
// --------------------------------------------------

void test_spread() {

    // Equal best bid and ask
    {
        OrderBook book;

        assert(book.add_order(make_order(1101, Side::Buy, 100, 10)));
        assert(book.add_order(make_order(1102, Side::Sell, 100, 10)));

        assert(*book.spread() == 0);
    }

    // Crossed order book
    {
        OrderBook book;

        assert(book.add_order(make_order(1201, Side::Buy, 110, 10)));
        assert(book.add_order(make_order(1202, Side::Sell, 105, 10)));

        assert(*book.spread() == -5);
    }

    std::cout << "[PASS] Spread edge cases\n";
}


// --------------------------------------------------
// TEST 12: Display Functions
// --------------------------------------------------

void test_display() {

    OrderBook book;

    assert(book.add_order(make_order(1301, Side::Buy, 100, 10)));
    assert(book.add_order(make_order(1302, Side::Sell, 110, 20)));

    // These should execute without modifying the book
    book.display_order_book();
    book.display_detailed_order_book();

    assert(book.order_count() == 2);
    assert(*book.best_bid() == 100);
    assert(*book.best_ask() == 110);

    std::cout << "[PASS] Display functions\n";
}


// --------------------------------------------------
// MAIN TEST RUNNER
// --------------------------------------------------

int main() {

    std::cout << "\n========== ORDER BOOK TESTS ==========\n\n";

    test_empty_order_book();
    test_add_orders();
    test_price_priority();
    // test_fifo_priority();
    test_duplicate_order_id();
    test_invalid_orders();
    test_cancel_orders();
    test_cancel_nonexistent_order();
    test_remove_empty_price_level();
    test_market_statistics();
    test_spread();
    test_display();

    std::cout << "\n========== ALL TESTS PASSED ==========\n";

    return 0;
}
