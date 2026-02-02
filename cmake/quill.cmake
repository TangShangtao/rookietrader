add_custom_target(
    quill
    COMMAND cmake -S . -B target/build && cmake --build target/build && cmake --install target/build --prefix target/install
    WORKING_DIRECTORY ${3rdparty_path}/quill
    COMMENT "build quill"
)
set(3rdparty_include ${3rdparty_include} ${3rdparty_path}/quill/target/install/include/)
