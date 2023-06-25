#include "../CTPGateway.h"
#include "Defines/TradeDefs/Entrust.hpp"
Entrust* CTPGateway::entrust_to_my(CThostFtdcInputOrderField* entrustField)
{
    ContractInfo* contract = m_cmgr->get_contract(entrustField->InstrumentID, entrustField->ExchangeID);
    if (contract == nullptr) return nullptr;
    Entrust* pret = Entrust::create(
        entrustField->InstrumentID, 
        entrustField->VolumeTotalOriginal, 
        entrustField->LimitPrice, 
        entrustField->ExchangeID
        );
    pret->set_contract_info(contract);
    pret->set_direction(direction_type_to_my(entrustField->Direction, entrustField->CombOffsetFlag[0]));
    pret->set_offset_type(offset_type_to_my(entrustField->CombOffsetFlag[0]));
    if (entrustField->TimeCondition == THOST_FTDC_TC_GFD)
	{
		pret->set_order_flag(OF_NOR);
	}
	else if (entrustField->TimeCondition == THOST_FTDC_TC_IOC)
	{
		if (entrustField->VolumeCondition == THOST_FTDC_VC_AV || entrustField->VolumeCondition == THOST_FTDC_VC_MV)
			pret->set_order_flag(OF_FAK);
		else
			pret->set_order_flag(OF_FOK);
	}
    generate_entrustID(pret->get_entrustID(), m_uFrontID, m_uSessionID, strtoul(entrustField->OrderRef, nullptr, 10));
	// const char* usertag = m_eidCache.get(pRet->getEntrustID());
	// if (strlen(usertag) > 0)
	// 	pRet->setUserTag(usertag);   
    return pret; 
}
