add_executable(json_sanity ${PROJECT_SOURCE_DIR}/src/json/json_sanity_main.cpp)
target_link_libraries(json_sanity PRIVATE highperliquid)

add_executable(book_sanity ${PROJECT_SOURCE_DIR}/src/book/book_sanity_main.cpp)
target_link_libraries(book_sanity PRIVATE highperliquid)

add_executable(pipeline_sanity ${PROJECT_SOURCE_DIR}/src/book/pipeline_sanity_main.cpp)
target_link_libraries(pipeline_sanity PRIVATE highperliquid)

add_executable(latency_sanity ${PROJECT_SOURCE_DIR}/src/latency/latency_sanity_main.cpp)
target_link_libraries(latency_sanity PRIVATE highperliquid)

add_executable(pipeline_latency_sanity ${PROJECT_SOURCE_DIR}/src/latency/pipeline_latency_sanity_main.cpp)
target_link_libraries(pipeline_latency_sanity PRIVATE highperliquid)
target_compile_definitions(pipeline_latency_sanity PRIVATE HPL_ENABLE_LATENCY=1)

add_executable(trading_model_sanity ${PROJECT_SOURCE_DIR}/src/trading/trading_model_sanity_main.cpp)
target_link_libraries(trading_model_sanity PRIVATE highperliquid)

add_executable(place_order_payload_sanity ${PROJECT_SOURCE_DIR}/src/execution/place_order_payload_sanity_main.cpp)
target_link_libraries(place_order_payload_sanity PRIVATE highperliquid)

add_executable(order_updates_sanity ${PROJECT_SOURCE_DIR}/src/json/order_updates_sanity_main.cpp)
target_link_libraries(order_updates_sanity PRIVATE highperliquid)
target_compile_definitions(order_updates_sanity PRIVATE HPL_ORDER_UPDATES_DEBUG=1)

add_executable(user_fills_sanity ${PROJECT_SOURCE_DIR}/src/json/user_fills_sanity_main.cpp)
target_link_libraries(user_fills_sanity PRIVATE highperliquid)

add_executable(execution_pipeline_sanity ${PROJECT_SOURCE_DIR}/src/execution/execution_pipeline_sanity_main.cpp)
target_link_libraries(execution_pipeline_sanity PRIVATE highperliquid)

add_executable(risk_sanity ${PROJECT_SOURCE_DIR}/src/risk/risk_sanity_main.cpp)
target_link_libraries(risk_sanity PRIVATE highperliquid)

add_executable(tls_sanity ${PROJECT_SOURCE_DIR}/src/net/tls_sanity_main.cpp)
target_link_libraries(tls_sanity PRIVATE highperliquid)

add_executable(wss_handshake_sanity ${PROJECT_SOURCE_DIR}/src/ws/wss_handshake_sanity_main.cpp)
target_link_libraries(wss_handshake_sanity PRIVATE highperliquid)