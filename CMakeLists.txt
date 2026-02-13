cmake_minimum_required(VERSION 3.20)

project(
    highperliquid
    VERSION 0.1.0
    LANGUAGES CXX
)

include(CTest)
enable_testing()

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
        src/config/config_loader.cpp
        src/memory/pool.cpp
        src/net/tcp_socket.cpp
        src/ws/dummy_ws.cpp
)


option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
if (ENABLE_SANITIZERS)
    target_compile_options(highperliquid PRIVATE -fsanitize=address,undefined)
    target_link_options(highperliquid PRIVATE -fsanitize=address,undefined)
endif()

add_library(simdjson STATIC
    third_party/simdjson/simdjson.cpp
)

target_include_directories(simdjson
    SYSTEM PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simdjson
)

target_compile_features(simdjson PUBLIC cxx_std_20)

set_target_properties(simdjson PROPERTIES
    CXX_CLANG_TIDY ""
)

set_target_properties(simdjson PROPERTIES
    COMPILE_WARNING_AS_ERROR OFF
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(simdjson PRIVATE -Wno-error -Wno-everything)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(simdjson PRIVATE -Wno-error -w)
elseif (MSVC)
    target_compile_options(simdjson PRIVATE /WX- /W0)
endif()

add_library(hyperliquid_json
    src/json/l2_book_parser.cpp
    src/json/json_sanity_main.cpp
)

target_include_directories(hyperliquid_json
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hyperliquid_json
    PRIVATE
        simdjson
)

target_link_libraries(highperliquid
    PRIVATE
        hyperliquid_json
)

add_executable(json_sanity
    src/json/json_sanity_main.cpp
)

target_link_libraries(json_sanity
    PRIVATE
        highperliquid
        simdjson
)

add_executable(book_sanity
    src/book/book_sanity_main.cpp
)

target_link_libraries(book_sanity
    PRIVATE
        highperliquid
)

add_executable(pipeline_sanity
    src/book/pipeline_sanity_main.cpp
)

target_link_libraries(pipeline_sanity
    PRIVATE
        highperliquid
)

option(HPL_ENABLE_TESTS "Enable tests" ON)

if (HPL_ENABLE_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()