#pragma once
#include <atomic>
#include <string>

enum class Side
{
    BUY,
    SELL
};

struct Order
{
    static std::atomic<long> NEXT_ID;
    long                     id;
    Side                     side;
    double                   price;
    int                      quantity;

    Order(Side s, double p, int q)
        : id(NEXT_ID.fetch_add(1)), side(s), price(p), quantity(q)
    {}
};
