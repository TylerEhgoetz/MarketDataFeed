#pragma once
#include "Order.h"
#include <functional>
#include <mutex>
#include <queue>
#include <vector>

struct Trade
{
    long   buyId, sellId;
    double price;
    int    qty;
};

class MatchingEngine
{
public:
    MatchingEngine()
        : buys(), sells(), mtx()
    {}

    // Add a new order and attempt matching
    std::vector<Trade> addOrder(const Order& order);

private:
    // Highest‐price BUY first; earliest ID first on tie
    struct BuyCmp
    {
        bool operator()(const Order& a, const Order& b) const
        {
            if (a.price != b.price)
                return a.price < b.price;
            return a.id > b.id;
        }
    };
    // Lowest‐price SELL first; earliest ID first on tie
    struct SellCmp
    {
        bool operator()(const Order& a, const Order& b) const
        {
            if (a.price != b.price)
                return a.price > b.price;
            return a.id > b.id;
        }
    };

    std::priority_queue<Order, std::vector<Order>, BuyCmp>  buys;
    std::priority_queue<Order, std::vector<Order>, SellCmp> sells;
    std::mutex                                              mtx;
};
