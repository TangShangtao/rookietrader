//
// Created by root on 2025/9/29.
//

#pragma once
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "data_type.h"
#include "util/db.h"
namespace rk
{
	/// 订单管理系统, 本地维护数据(资金, 持仓, 委托, 成交)
	///
	struct TradeInfo;
	struct MarketInfo;
	class OMS
	{
	public:

		explicit OMS(
			const config_type::AccountConfig& account_config,
			db::Executor& db_writer
		);
		void set_trade_info(std::shared_ptr<TradeInfo> trade_info);
		void set_market_info(std::shared_ptr<MarketInfo> market_info);
		// trade
		data_type::OrderRef order_insert(const data_type::OrderReq& req);
		void order_cancel(data_type::OrderRef order_ref);
		// handler
		void handle_tick(const data_type::TickData& data);
		void handle_bar(const data_type::BarData& data);
		void handle_trade(const data_type::TradeData& data);
		void handle_cancel(const data_type::CancelData& data);
		void handle_error(const data_type::OrderError& data);

	private:
		std::shared_ptr<TradeInfo> _trade_info = std::make_shared<TradeInfo>();
		std::shared_ptr<MarketInfo> _market_info = std::make_shared<MarketInfo>();
		const config_type::AccountConfig& _account_config;
		db::Executor& _db_writer;
	};
};