#include "pyapi.h"
#include "pyutils.h"
#include "pyprotocol.h"

namespace py = pybind11;

PYBIND11_MODULE(pyrookietrader, m) 
{
    rookietrader::BindProtocol(m.def_submodule("Protocol"));
    rookietrader::BindApi(m.def_submodule("TradeApi"));
    rookietrader::BindUtils(m.def_submodule("Utils"));
    // rookietrader::Bind
}