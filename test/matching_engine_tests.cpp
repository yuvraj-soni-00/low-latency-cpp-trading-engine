
#include "../include/matching_engine.hpp"

#include <gtest/gtest.h>

// --------------------------------------------------
// HELPER FUNCTION
// --------------------------------------------------

using namespace trading;

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

class MatchingEngineTest : public ::testing::Test
{
protected:
    Engine engine;
};

// --------------------------------------------------
// 1. INITIAL STATE
// --------------------------------------------------

TEST_F(MatchingEngineTest, InitiallyEmpty)
{
    EXPECT_TRUE(engine.order_book().empty());

    EXPECT_EQ(engine.order_book().order_count(), 0);

    EXPECT_TRUE(engine.trading_history().empty());
}

// --------------------------------------------------
// 2. ORDERS WITHOUT OPPOSITE LIQUIDITY
// --------------------------------------------------

TEST_F(MatchingEngineTest, BuyOrderAcceptedWithoutSellOrders)
{
    auto result = engine.process_order(
        make_order(101, Side::Buy, 100, 10)
    );

    EXPECT_TRUE(result.accepted);
    EXPECT_TRUE(result.trades.empty());
    EXPECT_EQ(result.remaining_quantity, 10);

    EXPECT_EQ(engine.order_book().order_count(), 1);

    ASSERT_TRUE(engine.order_book().best_bid().has_value());
    EXPECT_EQ(*engine.order_book().best_bid(), 100);
}

TEST_F(MatchingEngineTest, SellOrderAcceptedWithoutBuyOrders)
{
    auto result = engine.process_order(
        make_order(102, Side::Sell, 110, 20)
    );

    EXPECT_TRUE(result.accepted);
    EXPECT_TRUE(result.trades.empty());
    EXPECT_EQ(result.remaining_quantity, 20);

    EXPECT_EQ(engine.order_book().order_count(), 1);

    ASSERT_TRUE(engine.order_book().best_ask().has_value());
    EXPECT_EQ(*engine.order_book().best_ask(), 110);
}

// --------------------------------------------------
// 3. PRICE COMPATIBILITY
// --------------------------------------------------

TEST_F(MatchingEngineTest, BuyDoesNotMatchWhenAskIsAboveLimit)
{
    auto sell = engine.process_order(
        make_order(201, Side::Sell, 110, 10)
    );

    ASSERT_TRUE(sell.accepted);

    auto buy = engine.process_order(
        make_order(202, Side::Buy, 105, 10)
    );

    EXPECT_TRUE(buy.accepted);
    EXPECT_TRUE(buy.trades.empty());
    EXPECT_EQ(buy.remaining_quantity, 10);

    EXPECT_EQ(engine.order_book().order_count(), 2);

    ASSERT_TRUE(engine.order_book().best_bid().has_value());
    ASSERT_TRUE(engine.order_book().best_ask().has_value());

    EXPECT_EQ(*engine.order_book().best_bid(), 105);
    EXPECT_EQ(*engine.order_book().best_ask(), 110);
}

TEST_F(MatchingEngineTest, SellDoesNotMatchWhenBidIsBelowLimit)
{
    auto buy = engine.process_order(
        make_order(203, Side::Buy, 100, 10)
    );

    ASSERT_TRUE(buy.accepted);

    auto sell = engine.process_order(
        make_order(204, Side::Sell, 105, 10)
    );

    EXPECT_TRUE(sell.accepted);
    EXPECT_TRUE(sell.trades.empty());
    EXPECT_EQ(sell.remaining_quantity, 10);

    EXPECT_EQ(engine.order_book().order_count(), 2);

    ASSERT_TRUE(engine.order_book().best_bid().has_value());
    ASSERT_TRUE(engine.order_book().best_ask().has_value());

    EXPECT_EQ(*engine.order_book().best_bid(), 100);
    EXPECT_EQ(*engine.order_book().best_ask(), 105);
}

// --------------------------------------------------
// 4. EXACT MATCHING
// --------------------------------------------------

TEST_F(MatchingEngineTest, ExactFillForIncomingBuy)
{
    auto sell = engine.process_order(
        make_order(301, Side::Sell, 110, 10)
    );

    ASSERT_TRUE(sell.accepted);

    auto buy = engine.process_order(
        make_order(302, Side::Buy, 110, 10)
    );

    ASSERT_TRUE(buy.accepted);

    ASSERT_EQ(buy.trades.size(), 1);

    EXPECT_EQ(buy.trades[0].buy_order_Id, 302);
    EXPECT_EQ(buy.trades[0].ask_order_Id, 301);

    EXPECT_EQ(buy.trades[0].price, 110);
    EXPECT_EQ(buy.trades[0].quantity, 10);

    EXPECT_EQ(buy.remaining_quantity, 0);

    EXPECT_TRUE(engine.order_book().empty());
}

TEST_F(MatchingEngineTest, ExactFillForIncomingSell)
{
    auto buy = engine.process_order(
        make_order(303, Side::Buy, 110, 10)
    );

    ASSERT_TRUE(buy.accepted);

    auto sell = engine.process_order(
        make_order(304, Side::Sell, 110, 10)
    );

    ASSERT_TRUE(sell.accepted);

    ASSERT_EQ(sell.trades.size(), 1);

    EXPECT_EQ(sell.trades[0].buy_order_Id, 303);
    EXPECT_EQ(sell.trades[0].ask_order_Id, 304);

    EXPECT_EQ(sell.trades[0].price, 110);
    EXPECT_EQ(sell.trades[0].quantity, 10);

    EXPECT_EQ(sell.remaining_quantity, 0);

    EXPECT_TRUE(engine.order_book().empty());
}

// --------------------------------------------------
// 5. EXECUTION PRICE
// --------------------------------------------------

TEST_F(MatchingEngineTest, BuyExecutesAtRestingSellPrice)
{
    auto sell = engine.process_order(
        make_order(401, Side::Sell, 110, 10)
    );

    ASSERT_TRUE(sell.accepted);

    // Buyer is willing to pay up to 120.
    auto buy = engine.process_order(
        make_order(402, Side::Buy, 120, 10)
    );

    ASSERT_EQ(buy.trades.size(), 1);

    // Execution should happen at resting order price.
    EXPECT_EQ(buy.trades[0].price, 110);
}

TEST_F(MatchingEngineTest, SellExecutesAtRestingBuyPrice)
{
    auto buy = engine.process_order(
        make_order(403, Side::Buy, 110, 10)
    );

    ASSERT_TRUE(buy.accepted);

    // Seller is willing to sell for 100.
    auto sell = engine.process_order(
        make_order(404, Side::Sell, 100, 10)
    );

    ASSERT_EQ(sell.trades.size(), 1);

    // Execution should happen at resting order price.
    EXPECT_EQ(sell.trades[0].price, 110);
}

// --------------------------------------------------
// 6. PARTIAL FILLS
// --------------------------------------------------

TEST_F(MatchingEngineTest, PartialFillLeavesRemainingSellQuantity)
{
    auto sell = engine.process_order(
        make_order(501, Side::Sell, 110, 10)
    );

    ASSERT_TRUE(sell.accepted);

    auto buy = engine.process_order(
        make_order(502, Side::Buy, 110, 4)
    );

    ASSERT_EQ(buy.trades.size(), 1);

    EXPECT_EQ(buy.trades[0].quantity, 4);
    EXPECT_EQ(buy.remaining_quantity, 0);

    // 10 - 4 = 6 remaining on the resting sell.
    EXPECT_EQ(engine.order_book().best_ask_quantity(), 6);
    EXPECT_EQ(engine.order_book().best_ask_order_count(), 1);
    EXPECT_EQ(engine.order_book().order_count(), 1);
}

TEST_F(MatchingEngineTest, PartialFillLeavesRemainingBuyQuantity)
{
    auto buy = engine.process_order(
        make_order(503, Side::Buy, 110, 10)
    );

    ASSERT_TRUE(buy.accepted);

    auto sell = engine.process_order(
        make_order(504, Side::Sell, 110, 4)
    );

    ASSERT_EQ(sell.trades.size(), 1);

    EXPECT_EQ(sell.trades[0].quantity, 4);
    EXPECT_EQ(sell.remaining_quantity, 0);

    EXPECT_EQ(engine.order_book().best_bid_quantity(), 6);
    EXPECT_EQ(engine.order_book().best_bid_order_count(), 1);
    EXPECT_EQ(engine.order_book().order_count(), 1);
}

// --------------------------------------------------
// 7. REMAINING INCOMING QUANTITY
// --------------------------------------------------

TEST_F(MatchingEngineTest, RemainingBuyQuantityIsAddedToOrderBook)
{
    auto sell = engine.process_order(
        make_order(601, Side::Sell, 105, 4)
    );

    ASSERT_TRUE(sell.accepted);

    auto buy = engine.process_order(
        make_order(602, Side::Buy, 110, 10)
    );

    ASSERT_EQ(buy.trades.size(), 1);

    EXPECT_EQ(buy.trades[0].quantity, 4);

    // Incoming order had 10, only 4 were executed.
    EXPECT_EQ(buy.remaining_quantity, 6);

    // Remaining 6 should be inserted as a Buy order.
    EXPECT_EQ(engine.order_book().order_count(), 1);

    ASSERT_TRUE(engine.order_book().best_bid().has_value());
    EXPECT_EQ(*engine.order_book().best_bid(), 110);

    EXPECT_EQ(engine.order_book().best_bid_quantity(), 6);
    EXPECT_FALSE(engine.order_book().best_ask().has_value());
}

TEST_F(MatchingEngineTest, RemainingSellQuantityIsAddedToOrderBook)
{
    auto buy = engine.process_order(
        make_order(603, Side::Buy, 110, 4)
    );

    ASSERT_TRUE(buy.accepted);

    auto sell = engine.process_order(
        make_order(604, Side::Sell, 105, 10)
    );

    ASSERT_EQ(sell.trades.size(), 1);

    EXPECT_EQ(sell.trades[0].quantity, 4);
    EXPECT_EQ(sell.remaining_quantity, 6);

    EXPECT_EQ(engine.order_book().order_count(), 1);

    ASSERT_TRUE(engine.order_book().best_ask().has_value());
    EXPECT_EQ(*engine.order_book().best_ask(), 105);

    EXPECT_EQ(engine.order_book().best_ask_quantity(), 6);
    EXPECT_FALSE(engine.order_book().best_bid().has_value());
}

// --------------------------------------------------
// 8. MULTIPLE MATCHES
// --------------------------------------------------

TEST_F(MatchingEngineTest, IncomingBuyMatchesMultipleSellOrders)
{
    // Resting Sell orders at different price levels.
    ASSERT_TRUE(
        engine.process_order(
            make_order(701, Side::Sell, 105, 3)
        ).accepted
    );

    ASSERT_TRUE(
        engine.process_order(
            make_order(702, Side::Sell, 110, 4)
        ).accepted
    );

    auto buy = engine.process_order(
        make_order(703, Side::Buy, 115, 6)
    );

    ASSERT_EQ(buy.trades.size(), 2);

    EXPECT_EQ(buy.trades[0].ask_order_Id, 701);
    EXPECT_EQ(buy.trades[0].price, 105);
    EXPECT_EQ(buy.trades[0].quantity, 3);

    EXPECT_EQ(buy.trades[1].ask_order_Id, 702);
    EXPECT_EQ(buy.trades[1].price, 110);
    EXPECT_EQ(buy.trades[1].quantity, 3);

    EXPECT_EQ(buy.remaining_quantity, 0);

    // Second resting order has 4 - 3 = 1 remaining.
    EXPECT_EQ(engine.order_book().best_ask_quantity(), 1);
    EXPECT_EQ(engine.order_book().order_count(), 1);
}

TEST_F(MatchingEngineTest, IncomingSellMatchesMultipleBuyOrders)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(704, Side::Buy, 105, 3)
        ).accepted
    );

    ASSERT_TRUE(
        engine.process_order(
            make_order(705, Side::Buy, 110, 4)
        ).accepted
    );

    auto sell = engine.process_order(
        make_order(706, Side::Sell, 100, 6)
    );

    ASSERT_EQ(sell.trades.size(), 2);

    EXPECT_EQ(sell.trades[0].buy_order_Id, 705);
    EXPECT_EQ(sell.trades[0].price, 110);
    EXPECT_EQ(sell.trades[0].quantity, 4);

    EXPECT_EQ(sell.trades[1].buy_order_Id, 704);
    EXPECT_EQ(sell.trades[1].price, 105);
    EXPECT_EQ(sell.trades[1].quantity, 2);

    EXPECT_EQ(sell.remaining_quantity, 0);

    EXPECT_EQ(engine.order_book().best_bid_quantity(), 1);
    EXPECT_EQ(engine.order_book().order_count(), 1);
}

// --------------------------------------------------
// 9. PRICE PRIORITY
// --------------------------------------------------

TEST_F(MatchingEngineTest, BetterSellPriceExecutesFirst)
{
    // Insert the worse price first.
    ASSERT_TRUE(
        engine.process_order(
            make_order(801, Side::Sell, 110, 5)
        ).accepted
    );

    ASSERT_TRUE(
        engine.process_order(
            make_order(802, Side::Sell, 105, 5)
        ).accepted
    );

    auto buy = engine.process_order(
        make_order(803, Side::Buy, 115, 10)
    );

    ASSERT_EQ(buy.trades.size(), 2);

    // Lowest ask must execute first.
    EXPECT_EQ(buy.trades[0].ask_order_Id, 802);
    EXPECT_EQ(buy.trades[0].price, 105);

    EXPECT_EQ(buy.trades[1].ask_order_Id, 801);
    EXPECT_EQ(buy.trades[1].price, 110);
}

TEST_F(MatchingEngineTest, BetterBuyPriceExecutesFirst)
{
    // Insert the worse price first.
    ASSERT_TRUE(
        engine.process_order(
            make_order(804, Side::Buy, 100, 5)
        ).accepted
    );

    ASSERT_TRUE(
        engine.process_order(
            make_order(805, Side::Buy, 110, 5)
        ).accepted
    );

    auto sell = engine.process_order(
        make_order(806, Side::Sell, 95, 10)
    );

    ASSERT_EQ(sell.trades.size(), 2);

    // Highest bid must execute first.
    EXPECT_EQ(sell.trades[0].buy_order_Id, 805);
    EXPECT_EQ(sell.trades[0].price, 110);

    EXPECT_EQ(sell.trades[1].buy_order_Id, 804);
    EXPECT_EQ(sell.trades[1].price, 100);
}

// --------------------------------------------------
// 10. FIFO PRIORITY
// --------------------------------------------------

TEST_F(MatchingEngineTest, SellOrdersAtSamePriceFollowFifo)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(901, Side::Sell, 110, 2)
        ).accepted
    );

    ASSERT_TRUE(
        engine.process_order(
            make_order(902, Side::Sell, 110, 3)
        ).accepted
    );

    auto buy = engine.process_order(
        make_order(903, Side::Buy, 110, 4)
    );

    ASSERT_EQ(buy.trades.size(), 2);

    // First submitted Sell order executes first.
    EXPECT_EQ(buy.trades[0].ask_order_Id, 901);
    EXPECT_EQ(buy.trades[0].quantity, 2);

    EXPECT_EQ(buy.trades[1].ask_order_Id, 902);
    EXPECT_EQ(buy.trades[1].quantity, 2);

    // Second order has 1 remaining.
    EXPECT_EQ(engine.order_book().best_ask_quantity(), 1);
    EXPECT_EQ(engine.order_book().best_ask_order_count(), 1);
}

TEST_F(MatchingEngineTest, BuyOrdersAtSamePriceFollowFifo)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(904, Side::Buy, 110, 2)
        ).accepted
    );

    ASSERT_TRUE(
        engine.process_order(
            make_order(905, Side::Buy, 110, 3)
        ).accepted
    );

    auto sell = engine.process_order(
        make_order(906, Side::Sell, 110, 4)
    );

    ASSERT_EQ(sell.trades.size(), 2);

    EXPECT_EQ(sell.trades[0].buy_order_Id, 904);
    EXPECT_EQ(sell.trades[0].quantity, 2);

    EXPECT_EQ(sell.trades[1].buy_order_Id, 905);
    EXPECT_EQ(sell.trades[1].quantity, 2);

    EXPECT_EQ(engine.order_book().best_bid_quantity(), 1);
    EXPECT_EQ(engine.order_book().best_bid_order_count(), 1);
}

// --------------------------------------------------
// 11. EXACT PRICE BOUNDARIES
// --------------------------------------------------

TEST_F(MatchingEngineTest, BuyMatchesWhenLimitEqualsAsk)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1001, Side::Sell, 110, 10)
        ).accepted
    );

    auto buy = engine.process_order(
        make_order(1002, Side::Buy, 110, 10)
    );

    ASSERT_EQ(buy.trades.size(), 1);
    EXPECT_EQ(buy.trades[0].price, 110);
}

TEST_F(MatchingEngineTest, SellMatchesWhenLimitEqualsBid)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1003, Side::Buy, 100, 10)
        ).accepted
    );

    auto sell = engine.process_order(
        make_order(1004, Side::Sell, 100, 10)
    );

    ASSERT_EQ(sell.trades.size(), 1);
    EXPECT_EQ(sell.trades[0].price, 100);
}

// --------------------------------------------------
// 12. INVALID PRICE TESTS
// --------------------------------------------------

class InvalidIncomingPriceTest
    : public ::testing::TestWithParam<Price>
{
};

TEST_P(InvalidIncomingPriceTest, RejectsInvalidIncomingPrice)
{
    Engine local_engine;

    auto result = local_engine.process_order(
        make_order(1101, Side::Buy, GetParam(), 10)
    );

    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.trades.empty());

    EXPECT_TRUE(local_engine.order_book().empty());
    EXPECT_TRUE(local_engine.trading_history().empty());
}

INSTANTIATE_TEST_SUITE_P(
    InvalidPrices,
    InvalidIncomingPriceTest,
    ::testing::Values(
        Price{0},
        Price{-5},
        Price{-1},
        Price{1},
        Price{102}
    )
);

// --------------------------------------------------
// 13. INVALID QUANTITY AND SIDE
// --------------------------------------------------

TEST_F(MatchingEngineTest, RejectsZeroQuantity)
{
    auto result = engine.process_order(
        make_order(1201, Side::Buy, 100, 0)
    );

    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.trades.empty());
    EXPECT_TRUE(engine.order_book().empty());
}

TEST_F(MatchingEngineTest, RejectsInvalidSide)
{
    auto result = engine.process_order(
        make_order(
            1202,
            static_cast<Side>(99),
            100,
            10
        )
    );

    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.trades.empty());
    EXPECT_TRUE(engine.order_book().empty());
}

// --------------------------------------------------
// 14. DUPLICATE ORDER ID
// --------------------------------------------------

TEST_F(MatchingEngineTest, RejectsDuplicateActiveOrderId)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1301, Side::Sell, 110, 10)
        ).accepted
    );

    // Same ID, opposite side, and crossing price.
    auto result = engine.process_order(
        make_order(1301, Side::Buy, 110, 10)
    );

    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.trades.empty());

    // Original resting Sell order must remain untouched.
    EXPECT_EQ(engine.order_book().order_count(), 1);
    EXPECT_EQ(engine.order_book().best_ask_quantity(), 10);

    EXPECT_TRUE(engine.trading_history().empty());
}

// --------------------------------------------------
// 15. INVALID ORDER MUST NOT EXECUTE
// --------------------------------------------------

TEST_F(MatchingEngineTest, InvalidIncomingOrderDoesNotMatchRestingOrder)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1401, Side::Sell, 110, 10)
        ).accepted
    );

    // Invalid tick price, even though it crosses the resting ask.
    auto result = engine.process_order(
        make_order(1402, Side::Buy, 111, 10)
    );

    EXPECT_FALSE(result.accepted);
    EXPECT_TRUE(result.trades.empty());

    EXPECT_EQ(engine.order_book().order_count(), 1);
    EXPECT_EQ(engine.order_book().best_ask_quantity(), 10);

    EXPECT_TRUE(engine.trading_history().empty());
}

// --------------------------------------------------
// 16. CANCELLATION
// --------------------------------------------------

TEST_F(MatchingEngineTest, CancelUnmatchedBuyOrder)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1501, Side::Buy, 100, 10)
        ).accepted
    );

    EXPECT_TRUE(engine.cancel_order(1501));

    EXPECT_TRUE(engine.order_book().empty());
    EXPECT_EQ(engine.order_book().order_count(), 0);
}

TEST_F(MatchingEngineTest, CancelUnmatchedSellOrder)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1502, Side::Sell, 110, 10)
        ).accepted
    );

    EXPECT_TRUE(engine.cancel_order(1502));

    EXPECT_TRUE(engine.order_book().empty());
}

TEST_F(MatchingEngineTest, CancelNonexistentOrder)
{
    EXPECT_FALSE(engine.cancel_order(9999));
}

TEST_F(MatchingEngineTest, CannotCancelFullyFilledOrder)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1503, Side::Sell, 110, 10)
        ).accepted
    );

    auto result = engine.process_order(
        make_order(1504, Side::Buy, 110, 10)
    );

    ASSERT_EQ(result.trades.size(), 1);

    EXPECT_FALSE(engine.cancel_order(1503));
    EXPECT_FALSE(engine.cancel_order(1504));

    EXPECT_TRUE(engine.order_book().empty());
}

// --------------------------------------------------
// 17. TRADE HISTORY
// --------------------------------------------------

TEST_F(MatchingEngineTest, TradeIsRecordedInHistory)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1601, Side::Sell, 110, 10)
        ).accepted
    );

    auto result = engine.process_order(
        make_order(1602, Side::Buy, 110, 10)
    );

    ASSERT_EQ(result.trades.size(), 1);

    const auto& history = engine.trading_history();

    ASSERT_EQ(history.size(), 1);

    EXPECT_EQ(history[0].buy_order_Id, 1602);
    EXPECT_EQ(history[0].ask_order_Id, 1601);
    EXPECT_EQ(history[0].price, 110);
    EXPECT_EQ(history[0].quantity, 10);
}

TEST_F(MatchingEngineTest, NoTradeIsRecordedWhenOrdersDoNotMatch)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1603, Side::Sell, 110, 10)
        ).accepted
    );

    auto result = engine.process_order(
        make_order(1604, Side::Buy, 100, 10)
    );

    EXPECT_TRUE(result.trades.empty());
    EXPECT_TRUE(engine.trading_history().empty());
}

TEST_F(MatchingEngineTest, TradeHistoryAccumulatesMultipleTrades)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1605, Side::Sell, 100, 2)
        ).accepted
    );

    auto first = engine.process_order(
        make_order(1606, Side::Buy, 100, 2)
    );

    ASSERT_EQ(first.trades.size(), 1);

    ASSERT_TRUE(
        engine.process_order(
            make_order(1607, Side::Sell, 105, 3)
        ).accepted
    );

    auto second = engine.process_order(
        make_order(1608, Side::Buy, 105, 3)
    );

    ASSERT_EQ(second.trades.size(), 1);

    const auto& history = engine.trading_history();

    ASSERT_EQ(history.size(), 2);

    EXPECT_EQ(history[0].price, 100);
    EXPECT_EQ(history[1].price, 105);
}

// --------------------------------------------------
// 18. ORDER ID REUSE AFTER CANCELLATION
// --------------------------------------------------

TEST_F(MatchingEngineTest, ReusesOrderIdAfterCancellation)
{
    ASSERT_TRUE(
        engine.process_order(
            make_order(1701, Side::Buy, 100, 10)
        ).accepted
    );

    ASSERT_TRUE(engine.cancel_order(1701));

    auto result = engine.process_order(
        make_order(1701, Side::Sell, 110, 20)
    );

    EXPECT_TRUE(result.accepted);
    EXPECT_EQ(engine.order_book().order_count(), 1);

    ASSERT_TRUE(engine.order_book().best_ask().has_value());
    EXPECT_EQ(*engine.order_book().best_ask(), 110);
}
