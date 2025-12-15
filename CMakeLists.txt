cmake_minimum_required(VERSION 3.10)
project(rookietrader)

set(CMAKE_CXX_STANDARD 20)
set(compile_options
    -Wall
    -fPIC
    -pthread
)
if (${CMAKE_BUILD_TYPE} MATCHES Debug)
    set(compile_options ${compile_options}
        -g -O0 -rdynamic
    )
elseif (${CMAKE_BUILD_TYPE} MATCHES Release)
    set(compile_options ${compile_options}
        -O3
    )
else ()
endif ()

# 三方库
set(3rdparty_path ${PROJECT_SOURCE_DIR}/3rdparty)
set(3rdparty_include)
set(3rdparty_src)
set(3rdparty_lib)
set(3rdparty_so)
set(3rdparty_compile_options)
file(GLOB 3rds cmake/*.cmake)
foreach (3rd ${3rds})
    include(${3rd})
endforeach ()
# 全局参数
set(compile_options ${compile_options} ${3rdparty_compile_options})
string(REPLACE ";" " " CMAKE_CXX_FLAGS "${compile_options}")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${3rdparty_include}
)
link_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib
    ${3rdparty_lib}
)
add_subdirectory(src)
