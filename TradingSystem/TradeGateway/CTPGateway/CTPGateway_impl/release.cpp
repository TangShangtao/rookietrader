#include "../CTPGateway.h"
int CTPGateway::release() 
{
    if (m_pCTPApi)
    {
        m_pCTPApi->Release();
        m_pCTPApi = nullptr;
    }
    if (m_ayOrders) m_ayOrders->clear();
    if (m_ayPosDetail) m_ayPosDetail->clear();
    if (m_mapPosition) m_mapPosition->clear();
    if ((m_ayTrades)) m_ayTrades->clear();  
    return 0;
}