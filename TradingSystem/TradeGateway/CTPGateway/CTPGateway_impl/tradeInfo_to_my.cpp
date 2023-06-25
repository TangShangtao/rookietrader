#include "../CTPGateway.h"
#include "Defines/TradeDefs/TradeInfo.hpp"
TradeInfo* CTPGateway::tradeInfo_to_my(CThostFtdcTradeField* tradeField)
{
    ContractInfo* contract = m_cmgr->get_contract(tradeField->InstrumentID, tradeField->ExchangeID);
    if (contract == nullptr) return nullptr;
    TradeInfo* pret = TradeInfo::create(tradeField->InstrumentID, tradeField->ExchangeID);
    pret->set_contract_info(contract);

    //日期转换: 业务日期、交易日、夜盘
    std::string strDate = tradeField->TradeDate;
    std::string strTime = tradeField->TradeTime;
    StrUtils::replace(strTime, ":", "");
    uint32_t uDate = strtoul(strDate.c_str(), nullptr, 10);
    uint64_t uTime = strtoull(strTime.c_str(), nullptr, 10);
    pret->set_trade_date(uDate);
    pret->set_trade_time(uTime); //TODO makeTime

    pret->set_volume(tradeField->Volume);
    pret->set_price(tradeField->Price);
    pret->set_trade_type((TradeType)tradeField->TradeType);
    double amount = contract->get_comm_info()->get_vol_scale() * tradeField->Volume * tradeField->Price;
    pret->set_amount(amount);
    pret->set_direction(direction_type_to_my(tradeField->Direction, tradeField->OffsetFlag));
    pret->set_offset_type(offset_type_to_my(tradeField->OffsetFlag));
    pret->set_tradeID(tradeField->TradeID);
    pret->set_ref_orderID(tradeField->OrderSysID);

	// const char* usertag = m_oidCache.get(StrUtil::trim(pRet->getRefOrder()).c_str());
	// if (strlen(usertag))
	// 	pRet->setUserTag(usertag);
    return pret;
}
