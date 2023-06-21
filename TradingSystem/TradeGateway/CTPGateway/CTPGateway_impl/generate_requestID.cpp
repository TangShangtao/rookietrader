#include "../CTPGateway.h"

uint32_t CTPGateway::generate_requestID()
{
    return m_uRequestID.fetch_add(1) + 1;
}