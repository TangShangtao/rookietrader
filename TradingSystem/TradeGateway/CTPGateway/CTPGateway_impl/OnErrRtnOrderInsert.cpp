#include "../CTPGateway.h"
#include "Defines/TradeDefs/Entrust.hpp"
#include "tools/Logger.h"
void CTPGateway::OnErrRtnOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo) 
{
    Entrust* pEntrust = entrust_to_my(pInputOrder);
    // if (pEn)
    // {
    //     m_sink->on_order(orderInfo);
    //     orderInfo->release();
    // }
    pEntrust->release();
    if (is_err_rspInfo(pRspInfo))
    {
        Logger::error("OnErrRtnOrderInsert: {}", pRspInfo->ErrorMsg);
    }
}