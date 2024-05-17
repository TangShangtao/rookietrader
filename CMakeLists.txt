cmake_minimum_required(VERSION 3.15)

project(rookietrader CXX)

set(root ${CMAKE_CURRENT_SOURCE_DIR})
set(EXECUTABLE_OUTPUT_PATH ${root}/bin)
set(LIBRARY_OUTPUT_PATH ${root}/lib)
add_compile_options(-fPIC)
# libs developed by rookietrader
include_directories(${root}/include)
link_directories(${root}/lib)
# libs developed by thirdparty
include_directories(${root}/thirdparty/include)
link_directories(${root}/thirdparty/lib)


add_subdirectory(src)

