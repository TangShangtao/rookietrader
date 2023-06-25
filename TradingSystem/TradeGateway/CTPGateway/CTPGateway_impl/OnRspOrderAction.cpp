#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::OnRspOrderAction(CThostFtdcInputOrderActionField *pInputOrderAction, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo))
    {
        Logger::error("CTPGateway::OnRspOrderAction: error, requestID: {}, errorID: {}, errorMsg: {}", nRequestID, pRspInfo->ErrorID, pRspInfo->ErrorMsg);
    }
    else
    {
        //TODO通知注册的listener
        Logger::info("on rsp order insert successfully");
    }

    

}