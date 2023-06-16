#pragma once
#include "../../Defines/ConstantDefs/Marcos.h"
#include "../../Defines/ConstantDefs/Types.h"
#include "../../Defines/DataDefs/DataClass.hpp"

#include <stdint.h>
//先声明，随后编译器在链接时会去寻找定义
NS_BEGIN
class TickData;
class BarPeriod;
NS_END;

USING_NS;

class IEventCallbacks
{
    virtual void handle_tick(const char* stdCode, TickData* curTick, uint32_t pxType) = 0;
    // virtual void handle_order_queue(const char* stdCode, OrdQueData* curOrdQue) {};
    // virtual void handle_order_detail(const char* stdCode, OrdDtlData* curOrdDtl) {};
    // virtual void handle_transaction(const char* stdCode, TransData* curTrans) {};
    virtual void handle_bar_close(const char* stdCode, BarPeriod period, uint32_t times, BarStruct* newBar) = 0;
    virtual void handle_schedule(uint32_t uDate, uint32_t uTime) = 0;
    // virtual void handle_section_end() {};

    virtual void handle_init() = 0;
    virtual void handle_session_begin(uint32_t curTDate) = 0;
    virtual void handle_session_end(uint32_t curTDate) = 0;
    virtual void handle_replay_done() {};

};
