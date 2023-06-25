#include "../CTPGateway.h"

PriceType CTPGateway::price_type_to_my(TThostFtdcOrderPriceTypeType priceType)
{
	if (priceType == THOST_FTDC_OPT_AnyPrice || priceType == THOST_FTDC_OPT_FiveLevelPrice)
		return PT_AnyPrice;
	else if (priceType == THOST_FTDC_OPT_LimitPrice)
		return PT_LimitPrice;
	else if (priceType == THOST_FTDC_OPT_BestPrice)
		return PT_BestPrice;
	else
		return PT_LastPrice;

}
