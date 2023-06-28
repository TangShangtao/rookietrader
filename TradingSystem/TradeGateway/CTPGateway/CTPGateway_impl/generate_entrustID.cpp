#include "../CTPGateway.h"
#include "Utils/StrUtils.hpp"
#include "string.h"
bool CTPGateway::generate_entrustID(char* buffer, int length) 
{
    if (buffer == nullptr || length == 0) return false;
    memset(buffer, 0, length);
    uint32_t new_order_ref = m_uOrderRef.fetch_add(1) + 1;
    StrUtils::format_to(buffer, "{:06d}#{:010d}#{:06d}", m_uFrontID, m_uSessionID, new_order_ref);
    return true;
}
