aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} src)
add_executable(test ${src})
target_link_libraries(test rookietrader)