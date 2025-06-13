#include "MatchingEngine.h"

// addOrder: insert a new order, match against the opposite book, return any
// resulting trades
std::vector<Trade> MatchingEngine::addOrder(const Order& order)
{
    std::lock_guard<std::mutex> lk(mtx);
    std::vector<Trade>          trades;
    Order                       incoming = order;

    // Lambda that attempts to match 'incoming' against 'book'.
    // Since each priority_queue already has its comparator baked in, we don't
    // pass a comparator here.
    auto matchAgainst                    = [&](auto& book)
    {
        while (incoming.quantity > 0 && !book.empty())
        {
            const Order top = book.top();

            // Determine if prices cross (BUY price >= SELL price) or (SELL
            // price <= BUY price)
            bool priceMatch = false;
            if (incoming.side == Side::BUY)
            {
                priceMatch = (incoming.price >= top.price);
            }
            else
            {
                priceMatch = (incoming.price <= top.price);
            }
            if (!priceMatch)
                break;

            // Execute as much as possible
            int    qty        = std::min(incoming.quantity, top.quantity);
            double tradePrice = top.price;

            // Record the trade
            trades.push_back(
                { incoming.side == Side::BUY ? incoming.id : top.id,
                  incoming.side == Side::BUY ? top.id : incoming.id,
                  tradePrice,
                  qty }
            );

            // Pop the top of the book and re‐insert remainder if any
            book.pop();
            if (top.quantity > qty)
            {
                Order remainder = top;
                remainder.quantity -= qty;
                book.push(remainder);
            }

            incoming.quantity -= qty;
        }
    };

    // Match: if this is a BUY, match against the sell book; else match against
    // the buy book
    if (incoming.side == Side::BUY)
    {
        matchAgainst(sells);
    }
    else
    {
        matchAgainst(buys);
    }

    // If there’s leftover quantity, place into its own book
    if (incoming.quantity > 0)
    {
        if (incoming.side == Side::BUY)
        {
            buys.push(incoming);
        }
        else
        {
            sells.push(incoming);
        }
    }

    return trades;
}
