#include "pyprotocol.h"

namespace rookietrader
{
void BindProtocol(const py::module& m)
{
    BindEnum(m);
    BindStruct(m);
}
void BindEnum(const py::module& m)
{
    py::enum_<EventType>(m, "EventType", py::arithmetic())
        .value("EventNone", EventType::EventNone)
        .value("EventMDReady", EventType::EventMDReady)
        .value("EventTDReady", EventType::EventTDReady)
        .value("EventTick", EventType::EventTick)
        .value("EventBar", EventType::EventBar)
        .value("EventOrder", EventType::EventOrder)
        .value("EventTrade", EventType::EventTrade)
        .export_values();
    py::enum_<RPCType>(m, "RPCType", py::arithmetic())
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
    py::enum_<ExchangeID>(m, "ExchangeID", py::arithmetic())
        .value("None", ExchangeID::None)
        .value("SHFE", ExchangeID::SHFE)
        .value("DCE", ExchangeID::DCE)
        .value("CZCE", ExchangeID::CZCE)
        .value("CFFEX", ExchangeID::CFFEX)
        .export_values();
    py::enum_<OrderDirection>(m, "OrderDirection", py::arithmetic())
        .value("None", OrderDirection::None)
        .value("Long", OrderDirection::Long)
        .value("Short", OrderDirection::Short)
        .export_values();
    py::enum_<Offset>(m, "Offset", py::arithmetic())
        .value("None", Offset::None)
        .value("Open", Offset::Open)
        .value("CloseTd", Offset::CloseTd)
        .value("CloseYd", Offset::CloseYd)
        .export_values();
    py::enum_<OrderStatus>(m, "OrderStatus", py::arithmetic())
        .value("None", OrderStatus::None)
        .value("Rejected", OrderStatus::Rejected)
        .value("NoTradedQueueing", OrderStatus::NoTradedQueueing)
        .value("NoTradedCancelled", OrderStatus::NoTradedCancelled)
        .value("PartlyTradedQueueing", OrderStatus::PartlyTradedQueueing)
        .value("PartlyTradedCancelled", OrderStatus::PartlyTradedCancelled)
        .value("AllTraded", OrderStatus::AllTraded)
        .export_values();
}
void BindStruct(const py::module& m)
{
    py::class_<EventHeader>(m, "EventHeader")
        .def(py::init<uint32_t, EventType>())
        .def_readonly("rpcID", &EventHeader::rpcID)
        .def_readonly("event", &EventHeader::event)
        .def_property_readonly("tradingDay", &EventHeader::GetTradingDay)
        .def_property_readonly("generateTime", &EventHeader::GetGenerateTime);
    
    py::class_<Tick, EventHeader>(m, "Tick")
        .def(py::init<uint32_t>())
        .def_property_readonly("updateTime", &Tick::GetUpdateTime)
        .def_property_readonly("instrumentID", &Tick::GetInstrumentID)
        .def_readonly("exchangeID", &Tick::exchangeID)
        .def_readonly("lastPrice", &Tick::lastPrice)
        .def_readonly("bidPrices", &Tick::bidPrices)
        .def_readonly("bidVolumes", &Tick::bidVolumes)
        .def_readonly("askPrices", &Tick::askPrices)
        .def_readonly("askVolumes", &Tick::askVolumes);    
}
};