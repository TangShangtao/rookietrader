file(GLOB_RECURSE src
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
)
add_executable(rk_terminal ${src})
# TODO 临时使用engine_impl
target_include_directories(rk_terminal PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/engine_impl/ ${CMAKE_CURRENT_SOURCE_DIR}/../../engine/)
target_link_libraries(rk_terminal PRIVATE rk_engine replxx)
target_link_options(rk_terminal PRIVATE "-Wl,--as-needed")
