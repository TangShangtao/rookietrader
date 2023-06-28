#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo))
    {
        Logger::error("OnRspOrderAction: order action error, requestID: {}, errorID: {}, errorMsg: {}", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    else
    {
        //TODO通知注册的listener
        Logger::info("OnRspOrderAction: order action executed successfully");
    }

    

}