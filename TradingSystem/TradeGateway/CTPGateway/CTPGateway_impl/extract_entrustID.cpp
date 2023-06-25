#include "../CTPGateway.h"
#include "Utils/StrUtils.hpp"
bool CTPGateway::extract_entrustID(const char* pEntrustID, uint32_t& uFrontID, uint32_t& uSessionID, uint32_t& uOrderRef)
{
    char buffer[64] = {0};
    char* p = buffer;
    StrUtils::my_strncpy(buffer, pEntrustID, sizeof(buffer));
    auto idx = StrUtils::find_first(p, "#");
    if (idx == std::string::npos)
    {
        return false;
    }
    p[idx] = '\0';
    uFrontID = strtoul(buffer, nullptr, 10);
    p += idx + 1;
    idx = StrUtils::find_first(p, "#");
    if (idx == std::string::npos)
    {
        return false;
    }
    p[idx] = '\0';
    uSessionID = strtoul(p, nullptr, 10);
    p += idx + 1;
    uOrderRef = strtoul(p, nullptr, 10);
    return true;
    
}