cmake_minimum_required(VERSION 3.20)

project(
    highperliquid
    VERSION 0.1.0
    LANGUAGES CXX
)

find_package(OpenSSL REQUIRED)

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
        src/net/tls_socket.cpp
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
    src/json/order_updates_parser.cpp
    src/json/user_fills_parser.cpp
)

target_include_directories(hyperliquid_json
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(hyperliquid_json
    PUBLIC
        simdjson
)

target_link_libraries(highperliquid
    PUBLIC
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

add_executable(latency_sanity
    src/latency/latency_sanity_main.cpp
)

target_link_libraries(latency_sanity
    PRIVATE
        highperliquid
)

add_executable(pipeline_latency_sanity
    src/latency/pipeline_latency_sanity_main.cpp
)

target_link_libraries(pipeline_latency_sanity
    PRIVATE
        highperliquid
)

target_compile_definitions(pipeline_latency_sanity
    PRIVATE
        HPL_ENABLE_LATENCY=1
)

option(HPL_ENABLE_BENCHMARKS "Enable benchmarks" OFF)

if (HPL_ENABLE_BENCHMARKS)
  add_subdirectory(bench)
endif()


option(HPL_ENABLE_TESTS "Enable tests" ON)

if (HPL_ENABLE_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

add_executable(trading_model_sanity
    src/trading/trading_model_sanity_main.cpp
)
target_link_libraries(trading_model_sanity PRIVATE highperliquid)

add_executable(place_order_payload_sanity
    src/execution/place_order_payload_sanity_main.cpp
)
target_link_libraries(place_order_payload_sanity PRIVATE highperliquid)

add_executable(order_updates_sanity
    src/json/order_updates_sanity_main.cpp
)
target_link_libraries(order_updates_sanity PRIVATE highperliquid)

target_compile_definitions(hyperliquid_json PRIVATE HPL_ORDER_UPDATES_DEBUG=1)

add_executable(user_fills_sanity
    src/json/user_fills_sanity_main.cpp
)
target_link_libraries(user_fills_sanity PRIVATE highperliquid)

add_executable(execution_pipeline_sanity
    src/execution/execution_pipeline_sanity_main.cpp
)
target_link_libraries(execution_pipeline_sanity PRIVATE highperliquid)

add_executable(risk_sanity
    src/risk/risk_sanity_main.cpp
)
target_link_libraries(risk_sanity PRIVATE highperliquid)

add_executable(submit_order_sanity
    src/execution/submit_order_sanity_main.cpp
)
target_link_libraries(submit_order_sanity PRIVATE highperliquid)

target_link_libraries(highperliquid PRIVATE OpenSSL::SSL OpenSSL::Crypto)

add_executable(tls_sanity
    src/net/tls_sanity_main.cpp
)
target_link_libraries(tls_sanity PRIVATE highperliquid)

target_sources(highperliquid PRIVATE
    src/ws/wss_client.cpp
)

add_executable(wss_handshake_sanity
    src/ws/wss_handshake_sanity_main.cpp
)
target_link_libraries(wss_handshake_sanity PRIVATE highperliquid)

add_executable(live_l2book
    src/examples/live_l2book_main.cpp
)
target_link_libraries(live_l2book PRIVATE highperliquid)