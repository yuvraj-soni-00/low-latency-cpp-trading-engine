
#include "order_book.hpp"

#include <gtest/gtest.h>

#include <limits>

using namespace trading;

// --------------------------------------------------
// HELPER FUNCTION
// --------------------------------------------------

Order make_order(
    OrderId id,
    Side side,
    Price price,
    Quantity quantity
)
{
    Order order{};

    order.orderid = id;
    order.side = side;
    order.price = price;
    order.quantity = quantity;

    return order;
}

// --------------------------------------------------
// TEST FIXTURE
// --------------------------------------------------

class OrderBookTest : public ::testing::Test
{
protected:
    OrderBook book;
};

// --------------------------------------------------
// 1. EMPTY ORDER BOOK
// --------------------------------------------------

TEST_F(OrderBookTest, InitiallyEmpty)
{
    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.order_count(), 0);

    EXPECT_FALSE(book.best_bid().has_value());
    EXPECT_FALSE(book.best_ask().has_value());
    EXPECT_FALSE(book.spread().has_value());

    EXPECT_EQ(book.best_bid_quantity(), 0);
    EXPECT_EQ(book.best_ask_quantity(), 0);

    EXPECT_EQ(book.best_bid_order_count(), 0);
    EXPECT_EQ(book.best_ask_order_count(), 0);
}

// --------------------------------------------------
// 2. ADD ORDERS
// --------------------------------------------------

TEST_F(OrderBookTest, AddBuyOrder)
{
    EXPECT_TRUE(
        book.add_order(make_order(101, Side::Buy, 100, 10))
    );

    EXPECT_FALSE(book.empty());
    EXPECT_EQ(book.order_count(), 1);
    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(*book.best_bid(), 100);
}

TEST_F(OrderBookTest, AddSellOrder)
{
    EXPECT_TRUE(
        book.add_order(make_order(102, Side::Sell, 110, 20))
    );

    EXPECT_FALSE(book.empty());
    EXPECT_EQ(book.order_count(), 1);
    ASSERT_TRUE(book.best_ask().has_value());
    EXPECT_EQ(*book.best_ask(), 110);
}

TEST_F(OrderBookTest, AddMultipleBuyAndSellOrders)
{
    EXPECT_TRUE(book.add_order(make_order(103, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.add_order(make_order(104, Side::Buy, 105, 20)));

    EXPECT_TRUE(book.add_order(make_order(105, Side::Sell, 110, 15)));
    EXPECT_TRUE(book.add_order(make_order(106, Side::Sell, 115, 25)));

    EXPECT_EQ(book.order_count(), 4);
    EXPECT_FALSE(book.empty());
}

// --------------------------------------------------
// 3. PRICE PRIORITY
// --------------------------------------------------

TEST_F(OrderBookTest, BestBidIsHighestBuyPrice)
{
    EXPECT_TRUE(book.add_order(make_order(201, Side::Buy, 95, 10)));
    EXPECT_TRUE(book.add_order(make_order(202, Side::Buy, 110, 20)));
    EXPECT_TRUE(book.add_order(make_order(203, Side::Buy, 100, 15)));
    EXPECT_TRUE(book.add_order(make_order(204, Side::Buy, 105, 25)));

    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(*book.best_bid(), 110);
}

TEST_F(OrderBookTest, BestAskIsLowestSellPrice)
{
    EXPECT_TRUE(book.add_order(make_order(301, Side::Sell, 125, 10)));
    EXPECT_TRUE(book.add_order(make_order(302, Side::Sell, 115, 20)));
    EXPECT_TRUE(book.add_order(make_order(303, Side::Sell, 120, 15)));
    EXPECT_TRUE(book.add_order(make_order(304, Side::Sell, 130, 25)));

    ASSERT_TRUE(book.best_ask().has_value());
    EXPECT_EQ(*book.best_ask(), 115);
}

// --------------------------------------------------
// 4. INVALID PRICE TESTS
// --------------------------------------------------

class InvalidPriceTest : public ::testing::TestWithParam<Price>
{
};

TEST_P(InvalidPriceTest, RejectsInvalidPrice)
{
    OrderBook local_book;

    const Price invalid_price = GetParam();

    EXPECT_FALSE(
        local_book.add_order(
            make_order(401, Side::Buy, invalid_price, 10)
        )
    );

    EXPECT_TRUE(local_book.empty());
    EXPECT_EQ(local_book.order_count(), 0);
}

INSTANTIATE_TEST_SUITE_P(
    InvalidPrices,
    InvalidPriceTest,
    ::testing::Values(
        Price{0},
        Price{-5},
        Price{-1},
        Price{1},
        Price{102}
    )
);

// --------------------------------------------------
// 5. OTHER INVALID ORDERS
// --------------------------------------------------

TEST_F(OrderBookTest, RejectsZeroQuantity)
{
    EXPECT_FALSE(
        book.add_order(make_order(501, Side::Buy, 100, 0))
    );

    EXPECT_TRUE(book.empty());
}

TEST_F(OrderBookTest, RejectsInvalidSide)
{
    EXPECT_FALSE(
        book.add_order(
            make_order(
                502,
                static_cast<Side>(99),
                100,
                10
            )
        )
    );

    EXPECT_TRUE(book.empty());
}

TEST_F(OrderBookTest, AcceptsMinimumValidTickPrice)
{
    EXPECT_TRUE(
        book.add_order(
            make_order(503, Side::Buy, TICK_SIZE, 10)
        )
    );

    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(*book.best_bid(), TICK_SIZE);
}

// --------------------------------------------------
// 6. DUPLICATE ORDER ID
// --------------------------------------------------

TEST_F(OrderBookTest, RejectsDuplicateOrderIdOnSameSide)
{
    EXPECT_TRUE(book.add_order(make_order(601, Side::Buy, 100, 10)));

    EXPECT_FALSE(book.add_order(make_order(601, Side::Buy, 105, 20)));

    EXPECT_EQ(book.order_count(), 1);
    EXPECT_EQ(*book.best_bid(), 100);
}

TEST_F(OrderBookTest, RejectsDuplicateOrderIdOnOppositeSide)
{
    EXPECT_TRUE(book.add_order(make_order(602, Side::Buy, 100, 10)));

    EXPECT_FALSE(book.add_order(make_order(602, Side::Sell, 110, 20)));

    EXPECT_EQ(book.order_count(), 1);
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST_F(OrderBookTest, InvalidOrdersDoNotChangeExistingBook)
{
    EXPECT_TRUE(book.add_order(make_order(603, Side::Buy, 100, 10)));

    EXPECT_FALSE(book.add_order(make_order(604, Side::Buy, 101, 50)));
    EXPECT_FALSE(book.add_order(make_order(605, Side::Sell, 110, 0)));

    EXPECT_EQ(book.order_count(), 1);
    EXPECT_EQ(*book.best_bid(), 100);
}

// --------------------------------------------------
// 7. CANCELLATION
// --------------------------------------------------

TEST_F(OrderBookTest, CancelBuyOrder)
{
    EXPECT_TRUE(book.add_order(make_order(701, Side::Buy, 100, 10)));

    EXPECT_TRUE(book.cancel_order(701));

    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.order_count(), 0);
    EXPECT_FALSE(book.best_bid().has_value());
}

TEST_F(OrderBookTest, CancelSellOrder)
{
    EXPECT_TRUE(book.add_order(make_order(702, Side::Sell, 110, 10)));

    EXPECT_TRUE(book.cancel_order(702));

    EXPECT_TRUE(book.empty());
    EXPECT_FALSE(book.best_ask().has_value());
}

TEST_F(OrderBookTest, CancelNonexistentOrder)
{
    EXPECT_TRUE(book.add_order(make_order(703, Side::Buy, 100, 10)));

    EXPECT_FALSE(book.cancel_order(999));

    EXPECT_EQ(book.order_count(), 1);
    EXPECT_EQ(*book.best_bid(), 100);
}

TEST_F(OrderBookTest, CannotCancelSameOrderTwice)
{
    EXPECT_TRUE(book.add_order(make_order(704, Side::Buy, 100, 10)));

    EXPECT_TRUE(book.cancel_order(704));
    EXPECT_FALSE(book.cancel_order(704));

    EXPECT_TRUE(book.empty());
}

// --------------------------------------------------
// 8. PRICE LEVEL REMOVAL
// --------------------------------------------------

TEST_F(OrderBookTest, CancelBestBidMovesToNextPriceLevel)
{
    EXPECT_TRUE(book.add_order(make_order(801, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.add_order(make_order(802, Side::Buy, 105, 20)));

    EXPECT_TRUE(book.cancel_order(802));

    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(*book.best_bid(), 100);
}

TEST_F(OrderBookTest, CancelBestAskMovesToNextPriceLevel)
{
    EXPECT_TRUE(book.add_order(make_order(803, Side::Sell, 110, 10)));
    EXPECT_TRUE(book.add_order(make_order(804, Side::Sell, 115, 20)));

    EXPECT_TRUE(book.cancel_order(803));

    ASSERT_TRUE(book.best_ask().has_value());
    EXPECT_EQ(*book.best_ask(), 115);
}

TEST_F(OrderBookTest, CancelOneOrderAtSharedPriceLevel)
{
    EXPECT_TRUE(book.add_order(make_order(805, Side::Buy, 105, 10)));
    EXPECT_TRUE(book.add_order(make_order(806, Side::Buy, 105, 20)));

    EXPECT_TRUE(book.cancel_order(805));

    EXPECT_EQ(book.order_count(), 1);
    EXPECT_EQ(book.best_bid_order_count(), 1);
    EXPECT_EQ(book.best_bid_quantity(), 20);

    ASSERT_TRUE(book.best_bid().has_value());
    EXPECT_EQ(*book.best_bid(), 105);
}

// --------------------------------------------------
// 9. MARKET STATISTICS
// --------------------------------------------------

TEST_F(OrderBookTest, BestBidStatistics)
{
    EXPECT_TRUE(book.add_order(make_order(901, Side::Buy, 105, 10)));
    EXPECT_TRUE(book.add_order(make_order(902, Side::Buy, 105, 20)));
    EXPECT_TRUE(book.add_order(make_order(903, Side::Buy, 100, 100)));

    EXPECT_EQ(book.best_bid_quantity(), 30);
    EXPECT_EQ(book.best_bid_order_count(), 2);
}

TEST_F(OrderBookTest, BestAskStatistics)
{
    EXPECT_TRUE(book.add_order(make_order(904, Side::Sell, 110, 10)));
    EXPECT_TRUE(book.add_order(make_order(905, Side::Sell, 110, 20)));
    EXPECT_TRUE(book.add_order(make_order(906, Side::Sell, 115, 100)));

    EXPECT_EQ(book.best_ask_quantity(), 30);
    EXPECT_EQ(book.best_ask_order_count(), 2);
}

TEST_F(OrderBookTest, AskStatisticsWorkWithoutAnyBuyOrders)
{
    EXPECT_TRUE(book.add_order(make_order(907, Side::Sell, 110, 20)));

    EXPECT_EQ(book.best_ask_quantity(), 20);
    EXPECT_EQ(book.best_ask_order_count(), 1);
    EXPECT_EQ(book.best_bid_order_count(), 0);
}

TEST_F(OrderBookTest, BidStatisticsWorkWithoutAnySellOrders)
{
    EXPECT_TRUE(book.add_order(make_order(908, Side::Buy, 100, 20)));

    EXPECT_EQ(book.best_bid_quantity(), 20);
    EXPECT_EQ(book.best_bid_order_count(), 1);
    EXPECT_EQ(book.best_ask_order_count(), 0);
}

TEST_F(OrderBookTest, OrderCountTracksAllActiveOrders)
{
    EXPECT_TRUE(book.add_order(make_order(909, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.add_order(make_order(910, Side::Buy, 105, 20)));
    EXPECT_TRUE(book.add_order(make_order(911, Side::Sell, 110, 30)));

    EXPECT_EQ(book.order_count(), 3);

    EXPECT_TRUE(book.cancel_order(910));

    EXPECT_EQ(book.order_count(), 2);
}

// --------------------------------------------------
// 10. SPREAD EDGE CASES
// --------------------------------------------------

TEST_F(OrderBookTest, CalculatesNormalSpread)
{
    EXPECT_TRUE(book.add_order(make_order(1001, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.add_order(make_order(1002, Side::Sell, 110, 10)));

    ASSERT_TRUE(book.spread().has_value());
    EXPECT_EQ(*book.spread(), 10);
}

TEST_F(OrderBookTest, SpreadIsZeroForLockedBook)
{
    EXPECT_TRUE(book.add_order(make_order(1003, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.add_order(make_order(1004, Side::Sell, 100, 10)));

    ASSERT_TRUE(book.spread().has_value());
    EXPECT_EQ(*book.spread(), 0);
}

TEST_F(OrderBookTest, SpreadCanBeNegativeForCrossedBook)
{
    EXPECT_TRUE(book.add_order(make_order(1005, Side::Buy, 110, 10)));
    EXPECT_TRUE(book.add_order(make_order(1006, Side::Sell, 105, 10)));

    ASSERT_TRUE(book.spread().has_value());
    EXPECT_EQ(*book.spread(), -5);
}

TEST_F(OrderBookTest, NoSpreadWhenOnlyBuyOrdersExist)
{
    EXPECT_TRUE(book.add_order(make_order(1007, Side::Buy, 100, 10)));

    EXPECT_FALSE(book.spread().has_value());
}

TEST_F(OrderBookTest, NoSpreadWhenOnlySellOrdersExist)
{
    EXPECT_TRUE(book.add_order(make_order(1008, Side::Sell, 110, 10)));

    EXPECT_FALSE(book.spread().has_value());
}

// --------------------------------------------------
// 11. ADDITIONAL EDGE CASES
// --------------------------------------------------

TEST_F(OrderBookTest, CanReuseOrderIdAfterCancellation)
{
    EXPECT_TRUE(book.add_order(make_order(1101, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.cancel_order(1101));

    EXPECT_TRUE(book.add_order(make_order(1101, Side::Sell, 110, 20)));

    EXPECT_EQ(book.order_count(), 1);
    ASSERT_TRUE(book.best_ask().has_value());
    EXPECT_EQ(*book.best_ask(), 110);
}

TEST_F(OrderBookTest, HandlesLargeValidValues)
{
    const OrderId maximum_id =
        std::numeric_limits<OrderId>::max();

    const Price maximum_price =
        std::numeric_limits<Price>::max();

    const Price largest_tick_price =
        maximum_price - (maximum_price % TICK_SIZE);

    const Quantity maximum_quantity =
        std::numeric_limits<Quantity>::max();

    EXPECT_TRUE(
        book.add_order(
            make_order(
                maximum_id,
                Side::Buy,
                largest_tick_price,
                maximum_quantity
            )
        )
    );

    ASSERT_TRUE(book.best_bid().has_value());

    EXPECT_EQ(*book.best_bid(), largest_tick_price);
    EXPECT_EQ(book.best_bid_quantity(), maximum_quantity);
}

TEST_F(OrderBookTest, AllOrdersCanBeCancelled)
{
    EXPECT_TRUE(book.add_order(make_order(1102, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.add_order(make_order(1103, Side::Sell, 110, 20)));

    EXPECT_TRUE(book.cancel_order(1102));
    EXPECT_TRUE(book.cancel_order(1103));

    EXPECT_TRUE(book.empty());
    EXPECT_EQ(book.order_count(), 0);
}

// --------------------------------------------------
// 12. DISPLAY FUNCTIONS
// --------------------------------------------------

TEST_F(OrderBookTest, DisplayDoesNotModifyOrderBook)
{
    EXPECT_TRUE(book.add_order(make_order(1201, Side::Buy, 100, 10)));
    EXPECT_TRUE(book.add_order(make_order(1202, Side::Sell, 110, 20)));

    book.display_order_book();
    book.display_detailed_order_book();

    EXPECT_EQ(book.order_count(), 2);
    EXPECT_EQ(*book.best_bid(), 100);
    EXPECT_EQ(*book.best_ask(), 110);
}

