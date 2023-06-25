#include "../CTPGateway.h"

OffsetType CTPGateway::offset_type_to_my(TThostFtdcOffsetFlagType offsetType)
{
    if (offsetType == THOST_FTDC_OF_Open) return OT_Open;
    else if (offsetType == THOST_FTDC_OF_Close) return OT_Close;
    else if (offsetType == THOST_FTDC_OF_CloseToday) return OT_CloseToday;
    else if (offsetType == THOST_FTDC_OF_CloseYesterday) return OT_CloseYesterday;
    else return OT_ForceClose;

}