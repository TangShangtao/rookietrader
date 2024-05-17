#pragma once
#include "tools/utils.h"
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include "pybind11/stl.h"
namespace py = pybind11;

namespace rookietrader
{
void BindUtils(const py::module& m);

};