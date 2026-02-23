cmake_minimum_required(VERSION 3.20)
project(highperliquid VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(HPL_ENABLE_BENCHMARKS "Enable benchmarks" OFF)
option(HPL_ENABLE_TESTS "Enable tests" ON)

# Centralized options/warnings/tooling
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ProjectOptions)
include(Dependencies)

add_subdirectory(src)
add_subdirectory(apps)
add_subdirectory(examples)

if (HPL_ENABLE_BENCHMARKS)
  add_subdirectory(bench)
endif()

if (HPL_ENABLE_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()