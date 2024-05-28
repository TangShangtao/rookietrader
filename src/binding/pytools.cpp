#include "pytools.h"
#include "tools/database.h"
#include "tools/logger.h"
#include "tools/utils.h"

namespace rookietrader
{
void BindTools(const py::module& m)
{
    BindDatabase(m);
    BindLogger(m);
    BindUtils(m);
}

void BindDatabase(const py::module& m)
{

}

void BindLogger(const py::module& m)
{
    // py::class_<Logger>(m, "Logger")
    //     .def(py::init<std::string, std::string>())
    //     .def("debug", &Logger::debug)
    //     .def("info", &Logger::info)
    //     .def("warn", &Logger::warn)
    //     .def("error", &Logger::error);

}

void BindUtils(const py::module& m)
{
    
}
};