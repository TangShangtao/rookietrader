add_subdirectory(rk_terminal)

add_executable(test ${CMAKE_CURRENT_SOURCE_DIR}/test.cpp)
# TODO 临时使用engine_impl
target_include_directories(test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../engine/engine_impl/ ${CMAKE_CURRENT_SOURCE_DIR}/../engine/)
target_link_libraries(test PRIVATE rk_engine pqxx)
target_link_options(test PRIVATE "-Wl,--as-needed")
file(COPY templates DESTINATION ${EXECUTABLE_OUTPUT_PATH})