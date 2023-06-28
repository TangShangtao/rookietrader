#include "../CTPGateway.h"
#include "Defines/TradeDefs/Entrust.hpp"
#include "tools/Logger.h"
//报单录入错误响应
void CTPGateway::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo))
    {
        Logger::error("OnRspOrderInsert: order insert error: {}", pRspInfo->ErrorMsg);
        
    }
    Entrust* pEntrust = entrust_to_my(pInputOrder);

    // if (pEn)
    // {
    //     m_sink->on_order(orderInfo);
    //     orderInfo->release();
    // }
    if (pEntrust)
    {
        //m_sink->on_order(orderInfo);
        pEntrust->release();
    }
    



}