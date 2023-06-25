#include "../CTPGateway.h"
#include "Defines/TradeDefs/OrderInfo.hpp"
#include "tools/Logger.h"
//报单录入响应, 第一次为录入响应
void CTPGateway::OnRtnOrder(CThostFtdcOrderField *pOrder) 
{
    Logger::info("OnRtnOrder !!");
    OrderInfo* orderInfo = orderInfo_to_my(pOrder);
    // if (orderInfo)
    // {
    //     m_sink->on_order(orderInfo);
    //     orderInfo->release();
    // }
    Logger::info("{}, {}, {}, {}", orderInfo->get_code(), orderInfo->get_orderID(), orderInfo->get_vol_left(), orderInfo->get_vol_traded());
    orderInfo->release();
    
}