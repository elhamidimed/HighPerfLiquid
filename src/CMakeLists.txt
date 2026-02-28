add_library(highperliquid STATIC)

target_include_directories(highperliquid
  PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

hpl_apply_project_options(highperliquid)

target_link_libraries(highperliquid PUBLIC
  simdjson
  OpenSSL::SSL OpenSSL::Crypto
)

target_sources(highperliquid PRIVATE
  ${PROJECT_SOURCE_DIR}/src/dummy.cpp
  ${PROJECT_SOURCE_DIR}/src/core/timestamp.cpp
  ${PROJECT_SOURCE_DIR}/src/core/stdout_sink.cpp
  ${PROJECT_SOURCE_DIR}/src/config/config_loader.cpp
  ${PROJECT_SOURCE_DIR}/src/memory/pool.cpp
  ${PROJECT_SOURCE_DIR}/src/net/tcp_socket.cpp
  # + your TLS + WSS sources:
  ${PROJECT_SOURCE_DIR}/src/net/tls_socket.cpp
  ${PROJECT_SOURCE_DIR}/src/ws/wss_client.cpp

  # json parsers (currently implemented in .cpp files)
  ${PROJECT_SOURCE_DIR}/src/json/l2_book_parser.cpp
  ${PROJECT_SOURCE_DIR}/src/json/order_updates_parser.cpp
  ${PROJECT_SOURCE_DIR}/src/json/user_fills_parser.cpp
)

include(GNUInstallDirs)

set(HPL_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/HighPerfLiquid")

install(TARGETS highperliquid simdjson
  EXPORT HighPerfLiquidTargets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT HighPerfLiquidTargets
  FILE HighPerfLiquidTargets.cmake
  NAMESPACE HighPerfLiquid::
  DESTINATION ${HPL_INSTALL_CMAKEDIR}
)