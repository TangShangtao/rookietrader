set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rdparty/libpqxx libpqxx)
set(3rdparty_so ${3rdparty_so} pqxx)