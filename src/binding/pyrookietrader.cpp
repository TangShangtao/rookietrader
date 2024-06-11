#include "pyapi.h"
#include "pytools.h"
#include "pyprotocol.h"

namespace py = pybind11;

PYBIND11_MODULE(pyrookietrader, m) 
{
    rookietrader::BindProtocol(m.def_submodule("Protocol"));
    rookietrader::BindTools(m.def_submodule("Tools"));
    rookietrader::BindApi(m.def_submodule("TradeApi"));

}