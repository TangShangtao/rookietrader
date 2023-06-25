#include "../CTPGateway.h"

int CTPGateway::offset_type_to_CTP(OffsetType otype)
{
    if (otype == OT_Open) return THOST_FTDC_OF_Open;
    else if (otype == OT_Close) return THOST_FTDC_OF_Close;
    else if (otype == OT_CloseToday) return THOST_FTDC_OF_CloseToday;
    else if (otype == OT_CloseYesterday) return THOST_FTDC_OF_CloseYesterday;
    else return THOST_FTDC_OF_Close;

}