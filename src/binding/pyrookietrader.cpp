#include "pymdapi.h"
#include "pyprotocol.h"

namespace py = pybind11;

PYBIND11_MODULE(pyrookietrader, m) 
{
    rookietrader::BindProtocol(m.def_submodule("protocol"));
    // rookietrader::Bind
}