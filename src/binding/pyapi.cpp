#include "pyapi.h"



namespace rookietrader
{

class PyMDApi : public MDApi 
{
public:
    using MDApi::MDApi;
    void OnMDReady(const MDReady* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDApi, OnMDReady, event);
    }
    void OnTick(const Tick* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDApi, OnTick, event);
    }
    void OnPrepareMDRsp(const PrepareMDRsp* rsp) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDApi, OnPrepareMDRsp, rsp);
    }
    void OnSubTickRsp(const SubTickRsp* rsp) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDApi, OnSubTickRsp, rsp);
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
    mdapi.def(py::init<const std::string&>())
        .def("OnMDReady", &MDApi::OnMDReady)
        .def("OnTick", &MDApi::OnTick)

        .def("OnPrepareMDRsp", &MDApi::OnPrepareMDRsp)
        .def("OnSubTickRsp", &MDApi::OnSubTickRsp)

        .def("Init", &MDApi::Init)
        .def("SendPrepareMDReq", &MDApi::SendPrepareMDReq)
        .def("SendSubTickReq", &MDApi::SendSubTickReq);

}
void BindTDApi(const py::module& m)
{

}


};