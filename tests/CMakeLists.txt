add_executable(book_tests
    book_tests_main.cpp
)
target_link_libraries(book_tests PRIVATE highperliquid)
add_test(NAME book_tests COMMAND book_tests)

add_executable(pipeline_tests
    pipeline_tests_main.cpp
)
target_link_libraries(pipeline_tests PRIVATE highperliquid)
add_test(NAME pipeline_tests COMMAND pipeline_tests)