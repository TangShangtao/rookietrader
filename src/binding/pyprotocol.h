#pragma once
#include "protocol.h"
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include "pybind11/stl.h"
namespace py = pybind11;


namespace rookietrader
{
void BindProtocol(const py::module& m);

void BindEnum(const py::module& m);

void BindStruct(const py::module& m);

};
