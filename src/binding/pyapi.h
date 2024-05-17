#pragma once
#include "api/mdapi.h"
#include "api/tdapi.h"
#include "pybind11/pybind11.h"
#include "pybind11/numpy.h"
#include "pybind11/stl.h"
namespace py = pybind11;

namespace rookietrader
{
void BindApi(const py::module& m);

void BindMDApi(const py::module& m);

void BindTDApi(const py::module& m);

};