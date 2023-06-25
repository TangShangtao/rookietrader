#include "../CTPGateway.h"

int CTPGateway::price_type_to_CTP(PriceType priceType, bool isCFFEX /* = false */)
{
	if (PT_AnyPrice == priceType)
		return isCFFEX ? THOST_FTDC_OPT_FiveLevelPrice : THOST_FTDC_OPT_AnyPrice;
	else if (PT_LimitPrice == priceType)
		return THOST_FTDC_OPT_LimitPrice;
	else if (PT_BestPrice == priceType)
		return THOST_FTDC_OPT_BestPrice;
	else
		return THOST_FTDC_OPT_LastPrice;
}