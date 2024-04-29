cmake_minimum_required(VERSION 3.15)
add_subdirectory(pybind11)
include_directories(pybind11/include)
set(src 
    ${CMAKE_CURRENT_SOURCE_DIR}/pymdapi.cpp
)

pybind11_add_module(pyrookietrader ${src})
target_link_libraries(pyrookietrader PRIVATE rookietrader)