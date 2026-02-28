find_program(CLANG_TIDY_EXE NAMES clang-tidy)
if (CLANG_TIDY_EXE)
  set(CMAKE_CXX_CLANG_TIDY
    ${CLANG_TIDY_EXE};
    -warnings-as-errors=*
  )
endif()

# Apply warnings to all your targets via an INTERFACE library
add_library(hpl_options INTERFACE)

function(hpl_apply_project_options target)
  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE
      -Wall -Wextra -Wpedantic
      -Wconversion -Wsign-conversion
      -Wshadow
      -Werror
    )
  elseif (MSVC)
    target_compile_options(${target} PRIVATE /W4 /WX)
  endif()

  if (ENABLE_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(${target} PRIVATE -fsanitize=address,undefined)
    target_link_options(${target} PRIVATE -fsanitize=address,undefined)
  endif()
endfunction()