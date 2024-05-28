#include "pyapi.h"



namespace rookietrader
{

class PyMDApi : public MDApi 
{
public:
    using MDApi::MDApi;
    void OnMDApiStart() override
    {
        PYBIND11_OVERLOAD_PURE(void, MDApi, OnMDApiStart);
    }
    void OnMDReady(const MDReady* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDApi, OnMDReady, event);
    }
    void OnTick(const Tick* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDApi, OnTick, event);
    }
};

void BindApi(const py::module& m)
{
    BindMDApi(m);
    BindTDApi(m);
}
void BindMDApi(const py::module& m)
{
    py::class_<MDApi, PyMDApi> mdapi(m, "MDApi");
    mdapi.def(py::init<const std::string&,
                       const std::string&,
                       const std::string&,
                       const std::string&>())
        .def("OnMDApiStart", &MDApi::OnMDApiStart)
        .def("OnMDReady", &MDApi::OnMDReady)
        .def("OnTick", &MDApi::OnTick)

        .def("Subscribe", &MDApi::Subscribe)
        .def("Init", &MDApi::Init)
        .def("SendPrepareMDReq", &MDApi::SendPrepareMDReq)
        .def("SendSubTickReq", &MDApi::SendSubTickReq);
        
}
void BindTDApi(const py::module& m)
{

}


};