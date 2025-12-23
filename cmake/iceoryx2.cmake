add_custom_target(
    iceoryx2
    COMMAND cmake -S . -B target/ff/cc/build && cmake --build target/ff/cc/build && cmake --install target/ff/cc/build --prefix target/ff/install
    WORKING_DIRECTORY ${3rdparty_path}/iceoryx2
    COMMENT "build iceoryx2"
)
set(CMAKE_PREFIX_PATH  ${3rdparty_path}/iceoryx2/target/ff/install ${CMAKE_PREFIX_PATH})
set(3rdparty_include ${3rdparty_include} ${3rdparty_path}/iceoryx2/target/ff/install/include/iceoryx2/v0.7.0)
set(3rdparty_lib ${3rdparty_lib} ${3rdparty_path}/iceoryx2/target/ff/install/lib)
set(3rdparty_so ${3rdparty_so} iceoryx2_cxx iceoryx2_ffi_c)