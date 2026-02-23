find_package(OpenSSL REQUIRED)

add_library(simdjson STATIC
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simdjson/simdjson.cpp
)

target_include_directories(simdjson
  SYSTEM PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/simdjson
)

target_compile_features(simdjson PUBLIC cxx_std_20)

# Disable clang-tidy + warnings-as-errors for simdjson only
set_target_properties(simdjson PROPERTIES
  CXX_CLANG_TIDY ""
  COMPILE_WARNING_AS_ERROR OFF
)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  target_compile_options(simdjson PRIVATE -Wno-error -Wno-everything)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(simdjson PRIVATE -Wno-error -w)
elseif (MSVC)
  target_compile_options(simdjson PRIVATE /WX- /W0)
endif()