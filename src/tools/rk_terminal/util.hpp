#pragma once
#include "data_type.h"
#include <unordered_set>
namespace rk::rk_terminal
{
    std::vector<std::string> split_simple(const std::string& line, char delim = ',')
    {
        std::vector<std::string> fields;
        std::stringstream ss(line);
        std::string item;
        while (std::getline(ss, item, delim)) {
            fields.push_back(item);
        }
        return fields;
    }
    data_type::Symbol query_symbol(const char* c_input, std::unordered_set<data_type::Symbol>& symbols)
    {
        auto symbol_str = std::string(c_input);
        auto symbol = data_type::Symbol{symbol_str};
        if (
            auto it = std::find_if(
                symbols.begin(),
                symbols.end(),
                [&symbol_str](const data_type::Symbol &symbol) { return symbol.symbol == symbol_str; }
            );
            it != symbols.end()
            )
        {
            symbol = *it;
        }
        return symbol;
    }
    std::vector<std::vector<std::string>> to_row(const std::vector<data_type::PositionData>& data)
    {
        std::vector<std::vector<std::string>> res{};
        res.push_back({"symbol","product_class","long_position","long_frozen","long_pending","short_position","short_frozen","short_pending"});
        for (const auto& each : data)
        {
            res.push_back({
                              each.symbol.symbol.to_string(),std::string(magic_enum::enum_name(each.symbol.product_class)),
                              std::to_string(each.long_position.position),std::to_string(each.long_position.frozen),std::to_string(each.long_position.pending),
                              std::to_string(each.short_position.position),std::to_string(each.short_position.frozen),std::to_string(each.short_position.pending)
                          });
        }
        return res;
    }
    std::vector<std::vector<std::string>> to_row(const std::vector<data_type::OrderData>& data)
    {
        std::vector<std::vector<std::string>> res{};
        res.push_back({
                          "order_ref","symbol","product_class","limit_price","volume","direction","offset","req_time","traded_volume","canceled_volume",
                          "remain_volume"
                      });
        for (const auto& each : data)
        {
            res.push_back({
                              std::to_string(each.order_ref),
                              each.order_req.symbol.symbol.to_string(),std::string(magic_enum::enum_name(each.order_req.symbol.product_class)),
                              std::to_string(each.order_req.limit_price),std::to_string(each.order_req.volume),
                              std::string(magic_enum::enum_name(each.order_req.direction)),std::string(magic_enum::enum_name(each.order_req.offset)),
                              each.req_time.strftime(),std::to_string(each.traded_volume),std::to_string(each.remain_volume),
                              std::to_string(each.canceled_volume),
                          });
        }
        return res;
    }
    std::vector<std::vector<std::string>> to_row(const std::vector<data_type::TradeData>& data)
    {
        std::vector<std::vector<std::string>> res{};
        res.push_back({
                          "order_ref","trade_id","trade_price","trade_volume","trade_time","fee"
                      });
        for (const auto& each : data)
        {
            res.push_back({
                              std::to_string(each.order_ref),
                              each.trade_id.to_string(),
                              std::to_string(each.trade_price),std::to_string(each.trade_volume),
                              each.trade_time.strftime(),std::to_string(each.fee)
                          });
        }
        return res;
    }
    std::vector<std::vector<std::string>> to_row(const data_type::AccountData& data)
    {
        std::vector<std::vector<std::string>> res{};
        res.push_back({
                          "update_time","balance","market_value","margin","commission","available"
                      });
        res.push_back({
                          data.update_time.strftime(),
                          std::to_string(data.balance),
                          std::to_string(data.market_value),
                          std::to_string(data.margin),
                          std::to_string(data.commission),
                          std::to_string(data.available)
                      });
        return res;
    }
};