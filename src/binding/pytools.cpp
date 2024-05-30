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
    py::class_<Logger>(m, "Logger")
        .def(py::init<std::string, std::string>())
        .def("Debug", &Logger::Debug)
        .def("Info", &Logger::Info)
        .def("Warn", &Logger::Warn)
        .def("Error", &Logger::Error);
}

void BindUtils(const py::module& m)
{
    
}
};