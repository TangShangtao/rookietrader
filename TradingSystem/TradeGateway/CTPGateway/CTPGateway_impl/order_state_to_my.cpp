#include "../CTPGateway.h"

OrderState CTPGateway::order_state_to_my(TThostFtdcOrderStatusType orderState)
{
	if (orderState != THOST_FTDC_OST_Unknown)
		return (OrderState)orderState;
	else
		return OS_Submitting;

}
