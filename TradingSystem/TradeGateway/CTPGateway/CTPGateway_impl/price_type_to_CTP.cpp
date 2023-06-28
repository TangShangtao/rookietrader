#include "../CTPGateway.h"

int CTPGateway::price_type_to_CTP(PriceType priceType, bool isCFFEX /* = false */)
{
	// if (priceType == PT_AnyPrice)
	// 	return isCFFEX ? THOST_FTDC_OPT_FiveLevelPrice : THOST_FTDC_OPT_AnyPrice;
	// else if (priceType == PT_LimitPrice)
	// 	return THOST_FTDC_OPT_LimitPrice;
	// else if (priceType == PT_BestPrice)
	// 	return THOST_FTDC_OPT_BestPrice;
	// else
	// 	return THOST_FTDC_OPT_LastPrice;
	return THOST_FTDC_OPT_LimitPrice;
}