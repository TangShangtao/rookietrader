#include "../CTPGateway.h"

int CTPGateway::action_flag_to_CTP(ActionFlag flag)
{
    if (flag == AF_Cancel) return THOST_FTDC_AF_Delete;
    else return THOST_FTDC_AF_Modify;
    
}