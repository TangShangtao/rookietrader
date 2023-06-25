#include "../CTPGateway.h"


DirectionType CTPGateway::direction_type_to_my(TThostFtdcDirectionType dirType, TThostFtdcOffsetFlagType offsetType)
{
	if (THOST_FTDC_D_Buy == dirType)
		if (offsetType == THOST_FTDC_OF_Open)
			return DT_Long;
		else
			return DT_Short;
	else
		if (offsetType == THOST_FTDC_OF_Open)
			return DT_Short;
		else
			return DT_Long;

}
