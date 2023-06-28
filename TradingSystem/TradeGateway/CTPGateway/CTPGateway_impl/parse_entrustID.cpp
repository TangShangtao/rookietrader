#include "../CTPGateway.h"
#include "Utils/StrUtils.hpp"
#include "string.h"

bool CTPGateway::parse_entrustID(char* buffer, uint32_t uFrontID, uint32_t uSessionID, uint32_t uOrderRef)
{
    if (buffer == nullptr) return false;
    memset(buffer, 0, 64);
    StrUtils::format_to(buffer, "{:06d}#{:010d}#{:06d}", uFrontID, uSessionID, uOrderRef);
    return true;
}