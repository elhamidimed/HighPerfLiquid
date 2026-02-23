find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (CLANG_TIDY_EXE)
  set(CMAKE_CXX_CLANG_TIDY
    ${CLANG_TIDY_EXE};
    -warnings-as-errors=*
  )
endif()

# Apply warnings to all your targets via an INTERFACE library
add_library(hpl_options INTERFACE)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(hpl_options INTERFACE
    -Wall -Wextra -Wpedantic
    -Wconversion -Wsign-conversion
    -Wshadow
    -Werror
    -Wno-error=pedantic
  )
elseif (MSVC)
  target_compile_options(hpl_options INTERFACE /W4 /WX)
endif()

# Sanitizers (only for clang/gcc)
if (ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  target_compile_options(hpl_options INTERFACE -fsanitize=address,undefined)
  target_link_options(hpl_options INTERFACE -fsanitize=address,undefined)
endif()