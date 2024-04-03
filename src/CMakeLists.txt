message(${CMAKE_CURRENT_SOURCE_DIR})
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} src)
add_executable(main ${src})