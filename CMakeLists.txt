cmake_minimum_required(VERSION 3.20)

project(
    highperliquid
    VERSION 0.1.0
    LANGUAGES CXX
)

find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY
        ${CLANG_TIDY_EXE};
        -warnings-as-errors=*
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(highperliquid STATIC)

target_include_directories(highperliquid
    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
)

target_sources(highperliquid
    PRIVATE
        src/dummy.cpp
        src/core/timestamp.cpp
        src/core/stdout_sink.cpp
)


option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
if (ENABLE_SANITIZERS)
    target_compile_options(highperliquid PRIVATE -fsanitize=address,undefined)
    target_link_options(highperliquid PRIVATE -fsanitize=address,undefined)
endif()
