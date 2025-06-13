#include "MatchingEngine.h"
#include "Order.h"
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

PYBIND11_MODULE(engine, m)
{
    m.doc() = "Pybind11 wrapper for C++ MatchingEngine";

    // --- expose Side enum ---
    py::enum_<Side>(m, "Side")
        .value("BUY", Side::BUY)
        .value("SELL", Side::SELL)
        .export_values();

    // --- expose Order ---
    py::class_<Order>(m, "Order")
        .def(
            py::init<Side, double, int>(),
            py::arg("side"),
            py::arg("price"),
            py::arg("quantity")
        )
        .def_readonly("id", &Order::id)
        .def_readonly("side", &Order::side)
        .def_readwrite("price", &Order::price)
        .def_readwrite("quantity", &Order::quantity);

    // --- expose Trade ---
    py::class_<Trade>(m, "Trade")
        .def_readonly("buyId", &Trade::buyId)
        .def_readonly("sellId", &Trade::sellId)
        .def_readonly("price", &Trade::price)
        .def_readonly("qty", &Trade::qty);

    // --- expose MatchingEngine ---
    py::class_<MatchingEngine>(m, "MatchingEngine")
        .def(py::init<>())
        .def(
            "add_order",
            &MatchingEngine::addOrder,
            "Submit an Order and receive a list of Trade structs",
            py::arg("order")
        );
}
