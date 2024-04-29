#pragma once
#include "protocol.h"
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
namespace py = pybind11;


namespace rookietrader
{
void BindProtocol(py::module& m)
{
    py::enum_<EventType>(m, "EventType")
        .value("None", EventType::None)
        .value("MDReady", EventType::MDReady)
        .value("TDReady", EventType::TDReady)
        .value("Tick", EventType::Tick)
        .value("Bar", EventType::Bar)
        .value("Order", EventType::Order)
        .value("Trade", EventType::Trade)
        .export_values();
    py::enum_<RPCType>(m, "RPCType")
        .value("None", RPCType::None)
        .value("PrepareMD", RPCType::PrepareMD)
        .value("PrepareTD", RPCType::PrepareTD)
        .value("QryInstruments", RPCType::QryInstruments)
        .value("QryPosition", RPCType::QryPosition)
        .value("QryAccount", RPCType::QryAccount)
        .value("SubTick", RPCType::SubTick)
        .value("SubBar", RPCType::SubBar)
        .value("OrderInsert", RPCType::OrderInsert)
        .value("Cancel", RPCType::Cancel)
        .export_values();

    py::class_<EventHeader>(m, "EventHeader")
        .def(py::init<uint32_t, EventType>())
        .def_readwrite("rpcID", &EventHeader::rpcID)
        .def_readwrite("event", &EventHeader::event);
        // .def_readwrite("tradingDay", &EventHeader::tradingDay)
        // .def_readwrite("generateTime", &EventHeader::generateTime);
    
    py::class_<Tick, EventHeader>(m, "Tick")
        .def(py::init<uint32_t>())
        // .def_readwrite("updateTime", &Tick::updateTime)
        .def_readwrite("exchangeID", &Tick::exchangeID)
        .def_readwrite("lastPrice", &Tick::lastPrice);
        // .def_readwrite("bidPrices", &Tick::bidPrices)
        // .def_readwrite("bidVolumes", &Tick::bidVolumes)
        // .def_readwrite("askPrices", &Tick::askPrices)
        // .def_readwrite("askVolumes", &Tick::askVolumes);        
}


};
