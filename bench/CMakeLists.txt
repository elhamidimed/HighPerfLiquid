add_executable(bench_parser
    bench_parser_main.cpp
)
target_link_libraries(bench_parser PRIVATE highperliquid)

add_executable(bench_updater
    bench_updater_main.cpp
)
target_link_libraries(bench_updater PRIVATE highperliquid)

add_executable(bench_pipeline
    bench_pipeline_main.cpp
)
target_link_libraries(bench_pipeline PRIVATE highperliquid)