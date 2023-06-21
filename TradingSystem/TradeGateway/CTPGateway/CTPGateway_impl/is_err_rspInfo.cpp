#include "../CTPGateway.h"
bool CTPGateway::is_err_rspInfo(CThostFtdcRspInfoField *pRspInfo)
{
    if (pRspInfo == nullptr) return true;
    if (pRspInfo->ErrorID != 0) return true;
    return false;
}