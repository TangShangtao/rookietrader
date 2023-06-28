#include "../CTPGateway.h"
#include "tools/Logger.h"
void CTPGateway::OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) 
{
    Logger::info("OnRspError: {}", pRspInfo->ErrorMsg);
}