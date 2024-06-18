cmake_minimum_required(VERSION 3.2)

project(mgpu C CXX)

# Link against mgpu-cxx-opts to add common compiler flags to our own targets.
add_library(mgpu-cxx-opts INTERFACE)
target_compile_features(mgpu-cxx-opts INTERFACE cxx_std_20)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(mgpu-cxx-opts INTERFACE -Wall -Wextra -fno-exceptions)
endif()

add_subdirectory(external)
add_subdirectory(mgpu)

option(MGPU_BUILD_TESTS "Build test/demo applications" ON)

if(MGPU_BUILD_TESTS)
  add_subdirectory(test)
endif()