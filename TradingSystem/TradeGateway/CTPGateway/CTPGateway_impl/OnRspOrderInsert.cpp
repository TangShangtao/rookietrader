#include "../CTPGateway.h"
#include "tools/Logger.h"
//报单录入错误响应
void CTPGateway::OnRspOrderInsert(CThostFtdcInputOrderField *pInputOrder, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    if (is_err_rspInfo(pRspInfo))
    {
        Logger::error("on rsp order insert error: {}", pRspInfo->ErrorMsg);
    }
    else
    {
        //TODO通知注册的listener
        Logger::info("on rsp order insert successfully");
    }

}