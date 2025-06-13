#include "MatchingEngine.h"
#include "Order.h"
#include <iostream>

int main()
{
    MatchingEngine engine;

    // Simulate a few orders
    auto printTrades = [&](const std::vector<Trade>& trades)
    {
        for (auto& t : trades)
        {
            std::cout << "Trade: buy#" << t.buyId << " sell#" << t.sellId
                      << " price=" << t.price << " qty=" << t.qty << "\n";
        }
    };

    // New limit orders
    std::vector<Order> orders = {
        { Side::BUY,  100.0, 10 },
        { Side::SELL, 99.5,  5  },
        { Side::SELL, 100.0, 10 },
        { Side::BUY,  99.0,  20 }
    };

    for (auto& o : orders)
    {
        auto trades = engine.addOrder(o);
        printTrades(trades);
    }

    return 0;
}
