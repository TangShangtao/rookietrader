cmake_minimum_required(VERSION 3.15)

set(src 
    ${CMAKE_CURRENT_SOURCE_DIR}/mdapi.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/tdapi.cpp
)

target_sources(rookietrader PRIVATE ${src})