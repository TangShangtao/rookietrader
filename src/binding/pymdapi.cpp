// #include "api/mdapi.h"
// #include "pyprotocol.h"
// #include "pybind11/pybind11.h"
// namespace py = pybind11;
// namespace rookietrader
// {
// class PyMDSpi : public MDSpi
// {
// public:
//     using MDSpi::MDSpi;
//     // Event Callback
//     void OnMDReady(const MDReady* event) override
//     {
//         PYBIND11_OVERRIDE_PURE
//         (
//             void,
//             MDSpi,
//             OnMDReady,
//             event
//         );
//     }
//     void OnTick(const Tick* event) override
//     {
//         PYBIND11_OVERRIDE_PURE
//         (
//             void,
//             MDSpi,
//             OnTick,
//             event
//         );
//     }
//     // Rpc Callback
//     void OnPrepareMDRsp(const PrepareMDRsp* rsp) override
//     {
//         PYBIND11_OVERRIDE_PURE
//         (
//             void,
//             MDSpi,
//             OnPrepareMDRsp,
//             rsp
//         );
//     }
//     void OnSubTickRsp(const SubTickRsp* rsp) override
//     {
//         PYBIND11_OVERRIDE_PURE
//         (
//             void,
//             MDSpi,
//             OnSubTickRsp,
//             rsp
//         );
//     }
// };

// void test(MDSpi* spi)
// {
//     Tick tick(1);
//     tick.lastPrice = 100;
//     spi->OnTick(&tick);
// }

// PYBIND11_MODULE(pyrookietrader, m) {
//     m.doc() = "pybind11 rookietrader plugin"; // optional module docstring
//     BindProtocol(m);
//     py::class_<MDSpi, PyMDSpi>(m, "MDSpi")
//         .def(py::init<>())
//         .def("OnTick", &MDSpi::OnTick);
//     m.def("test", &test);
// }

// };
