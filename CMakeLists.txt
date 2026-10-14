cmake_minimum_required(VERSION 3.22)
# IDF's project macro resets its minimum policy version to 3.16. Keep the
# copied response-file toolchain variable during CMake compiler detection.
set(CMAKE_POLICY_DEFAULT_CMP0126 NEW)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(EXTRA_CMAKE_ARGS "-DCMAKE_POLICY_DEFAULT_CMP0126=NEW" APPEND)
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES CMAKE_TOOLCHAIN_FILE)
# The migration checkpoint preserves the old version; this is the MCP development build.
set(PROJECT_VER "0.5.1-mcp")
project(anan_stopwatch)
