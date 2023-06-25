#include "../CTPGateway.h"

//多开 buy ，多平 sell ，空开 sell ，空平 buy
int CTPGateway::direction_type_to_CTP(DirectionType dtype, OffsetType otype)
{
    if (dtype == DT_Long)
    {
        if (otype == OT_Open) return THOST_FTDC_D_Buy;
        else return THOST_FTDC_D_Sell;
    }
    else
    {
        if (otype == OT_Open) return THOST_FTDC_D_Sell;
        else return THOST_FTDC_D_Buy;
    }

}