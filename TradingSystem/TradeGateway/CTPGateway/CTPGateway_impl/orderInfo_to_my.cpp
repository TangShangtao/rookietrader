#include "../CTPGateway.h"
#include "Defines/TradeDefs/OrderInfo.hpp"
OrderInfo* CTPGateway::orderInfo_to_my(CThostFtdcOrderField* orderField)
{
    ContractInfo* contract = m_cmgr->get_contract(orderField->InstrumentID, orderField->ExchangeID);
    if (contract == nullptr) return nullptr;
    OrderInfo* pret = OrderInfo::create();
    if (pret == nullptr) return nullptr;
    pret->set_contract_info(contract);
    //cmgr与CTP字段的区别 ?
    pret->set_exchg(orderField->ExchangeID);
    pret->set_code(orderField->InstrumentID);
    
    //Entrust
    pret->set_direction(direction_type_to_my(orderField->Direction, orderField->CombOffsetFlag[0]));
    pret->set_price_type(price_type_to_my(orderField->OrderPriceType));
    pret->set_offset_type(offset_type_to_my(orderField->CombOffsetFlag[0]));
	if (orderField->TimeCondition == THOST_FTDC_TC_GFD)
	{
		pret->set_order_flag(OF_NOR);
	}
	else if (orderField->TimeCondition == THOST_FTDC_TC_IOC)
	{
		if (orderField->VolumeCondition == THOST_FTDC_VC_AV || orderField->VolumeCondition == THOST_FTDC_VC_MV)
			pret->set_order_flag(OF_FAK);
		else
			pret->set_order_flag(OF_FOK);
	}    
    pret->set_price(orderField->LimitPrice);
    pret->set_volume(orderField->VolumeTotalOriginal);
    
    
    //Order TODO夜盘转换、业务日期&交易日, CTP给的日期是业务日期还是交易日?
    uint32_t uDate = strtoul(orderField->InsertDate, nullptr, 10);
    std::string strTime = orderField->InsertTime;
    StrUtils::replace(strTime, ":", "");
    uint64_t uTime = strtoull(strTime.c_str(), nullptr, 10);
    pret->set_order_date(uDate);
    pret->set_order_time(uTime); //TODO makeTime
    
    
    
    pret->set_vol_traded(orderField->VolumeTraded);
    pret->set_vol_left(orderField->VolumeTotal);
    if (orderField->OrderSubmitStatus >= THOST_FTDC_OSS_InsertRejected)
        pret->set_rejected(true);
    pret->set_order_state(order_state_to_my(orderField->OrderStatus));
    parse_entrustID(pret->get_entrustID(), (uint32_t)orderField->FrontID, (uint32_t)orderField->SessionID, strtoul(orderField->OrderRef, nullptr, 10));
    //OrderType ? 
    pret->set_orderID(orderField->OrderSysID);
    pret->set_state_msg(orderField->StatusMsg);

	// const char* usertag = m_eidCache.get(pRet->getEntrustID());
	// if(strlen(usertag) == 0)
	// {
	// 	pRet->setUserTag(pRet->getEntrustID());
	// }
	// else
	// {
	// 	pRet->setUserTag(usertag);

	// 	if (strlen(pRet->getOrderID()) > 0)
	// 	{
	// 		m_oidCache.put(StrUtil::trim(pRet->getOrderID()).c_str(), usertag, 0, [this](const char* message) {
	// 			write_log(m_sink, LL_ERROR, message);
	// 		});
	// 	}
	// }
    return pret;
}