cmake_minimum_required(VERSION 3.22)
# IDF's project macro resets its minimum policy version to 3.16. Keep the
# copied response-file toolchain variable during CMake compiler detection.
set(CMAKE_POLICY_DEFAULT_CMP0126 NEW)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
idf_build_set_property(EXTRA_CMAKE_ARGS "-DCMAKE_POLICY_DEFAULT_CMP0126=NEW" APPEND)
list(APPEND CMAKE_TRY_COMPILE_PLATFORM_VARIABLES CMAKE_TOOLCHAIN_FILE)
# Preserve the original IDF app version through repository extraction.
# Future release version changes are separate from this migration.
set(PROJECT_VER "ec6bb29-dirty")
project(anan_stopwatch)
