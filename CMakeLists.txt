cmake_minimum_required(VERSION 3.20)
project(highperliquid VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(HPL_ENABLE_BENCHMARKS "Enable benchmarks" OFF)
option(HPL_ENABLE_TESTS "Enable tests" ON)

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/HighPerfLiquidConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/HighPerfLiquidConfig.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/HighPerfLiquid"
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/HighPerfLiquidConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/HighPerfLiquidConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/HighPerfLiquidConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/HighPerfLiquid"
)

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