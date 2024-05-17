#include "pyapi.h"



namespace rookietrader
{

class PyMDSpi : public MDSpi 
{
public:
    using MDSpi::MDSpi;
    void OnMDReady(const MDReady* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDSpi, OnMDReady, event);
    }
    void OnTick(const Tick* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDSpi, OnTick, event);
    }
    void OnPrepareMDRsp(const PrepareMDRsp* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDSpi, OnPrepareMDRsp, event);
    }
    void OnSubTickRsp(const SubTickRsp* event) override
    {
        PYBIND11_OVERLOAD_PURE(void, MDSpi, OnSubTickRsp, event);
    }
};

void BindApi(const py::module& m)
{
    BindMDApi(m);
    BindTDApi(m);
}
void BindMDApi(const py::module& m)
{
    py::class_<MDSpi, PyMDSpi> mdspi(m, "MDSpi");
    mdspi.def(py::init<>())
        .def("OnMDReady", &MDSpi::OnMDReady)
        .def("OnTick", &MDSpi::OnTick)
        .def("OnPrepareMDRsp", &MDSpi::OnPrepareMDRsp)
        .def("OnSubTickRsp", &MDSpi::OnSubTickRsp);

    py::class_<MDApi>(m, "MDApi")
        .def(py::init<const std::string&>())
        .def("Init", &MDApi::Init)
        .def("RegisterSpi", &MDApi::RegisterSpi)
        .def("SendPrepareMDReq", &MDApi::SendPrepareMDReq)
        .def("SendSubTickReq", &MDApi::SendSubTickReq);


}
void BindTDApi(const py::module& m)
{

}


};