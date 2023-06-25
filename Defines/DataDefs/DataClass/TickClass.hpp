#pragma once
#include "../DataStruct/TickStruct.hpp"
#include "Defines/BaseDefs/BaseObject.hpp"
#include "Defines/CommonDefs/ContractInfo.hpp"
#include <vector>
#include <algorithm>
NS_BEGIN

class TickData : public PoolObject<TickData>
{
public:
    TickStruct  m_tickStruct;
    ContractInfo* m_pContract;
public:
    TickData() : m_pContract(nullptr) {}
    static TickData* create(const char* stdCode)
    {
        TickData* pret = TickData::allocate();
        auto len = strlen(stdCode);
        memcpy(pret->m_tickStruct.code, stdCode, len);
        pret->m_tickStruct.code[len] = 0;
        
        return pret;
    }
    static TickData* create(TickStruct& tickStruct)
    {
        TickData* pret = allocate();
        memcpy(&pret->m_tickStruct, &tickStruct, sizeof(TickStruct));
        return pret;
    }
    void set_contractInfo(ContractInfo* contractInfo) { m_pContract = contractInfo; }

};

NS_END;