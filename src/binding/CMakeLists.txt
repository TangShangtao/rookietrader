cmake_minimum_required(VERSION 3.15)
add_subdirectory(pybind11)
include_directories(pybind11/include)

set(src 
    ${CMAKE_CURRENT_SOURCE_DIR}/pyapi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pyprotocol.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pytools.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/pyrookietrader.cpp
)

pybind11_add_module(pyrookietrader ${src})
target_link_libraries(pyrookietrader PRIVATE rookietrader pybind11::pybind11)
set_target_properties(pyrookietrader PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${root}/bin)
# TODO 路径修改
set(stubgen "/home/tangshangtao/anaconda3/bin/pybind11-stubgen")
add_custom_command(
    TARGET pyrookietrader POST_BUILD COMMAND 
    bash -c "PYTHONPATH=${root}/bin ${stubgen} pyrookietrader -o ."
    WORKING_DIRECTORY ${root}/bin
)