add_custom_target(
    NanoLog
    COMMAND 'make CXX_ARGS+="-fPIC"'
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/3rdparty/NanoLog/runtime
    COMMENT "build NanoLog"
)

set(3rdparty_include ${3rdparty_include} ${3rdparty_path}/NanoLog/runtime/)
set(3rdparty_lib ${3rdparty_lib} ${3rdparty_path}/NanoLog/runtime/)
set(3rdparty_so ${3rdparty_so} -lNanoLog -lrt -pthread)
set(3rdparty_compile_options ${3rdparty_compile_options} -Werror=format)
