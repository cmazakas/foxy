#
# Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
# or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/LeonineKing1199/foxy
#

cmake_minimum_required(VERSION 3.13)

project(
  foxy

  LANGUAGES
    CXX

  VERSION
    1.0.0

  DESCRIPTION
    "Session-based abstractions for Beast + URL parsing and pct-coding"
)

find_package(
  Boost 1.71
  CONFIG REQUIRED
    system
    date_time
)

find_package(OpenSSL REQUIRED)

if (UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON CACHE BOOL "")
endif()

find_package(Threads REQUIRED)

add_library(
  foxy

  include/foxy.hpp

  include/foxy/client_session.hpp
  include/foxy/code_point_iterator.hpp
  include/foxy/error.hpp
  include/foxy/log.hpp
  include/foxy/multi_stream.hpp
  include/foxy/pct_decode.hpp
  include/foxy/pct_encode.hpp
  include/foxy/proxy.hpp
  include/foxy/server_session.hpp
  include/foxy/session.hpp
  include/foxy/shared_handler_ptr.hpp
  include/foxy/type_traits.hpp
  include/foxy/uri_parts.hpp
  include/foxy/uri.hpp
  include/foxy/utf8.hpp
  include/foxy/utility.hpp

  include/foxy/detail/close_stream.hpp
  include/foxy/detail/export_connect_fields.hpp
  include/foxy/detail/has_token.hpp
  include/foxy/detail/relay.hpp
  include/foxy/detail/timed_op_wrapper_v2.hpp
  include/foxy/detail/tunnel.hpp

  include/foxy/impl/session.impl.hpp
  include/foxy/impl/shared_handler_ptr.impl.hpp

  include/foxy/impl/client_session/async_connect.impl.hpp
  include/foxy/impl/client_session/async_request.impl.hpp

  include/foxy/impl/server_session/async_detect_ssl.impl.hpp
  include/foxy/impl/server_session/async_handshake.impl.hpp

  include/foxy/impl/session/async_read.impl.hpp
  include/foxy/impl/session/async_read_header.impl.hpp
  include/foxy/impl/session/async_write.impl.hpp
  include/foxy/impl/session/async_write_header.impl.hpp

  src/log.cpp
  src/proxy.cpp
  src/uri_parts.cpp
  src/utility.cpp

  # TODO: someday make this work
  #
  # src/uri.cpp
)

target_compile_features(foxy PUBLIC cxx_generic_lambdas)

if (MSVC)
  # Win10
  #
  set(FOXY_WIN_ARCH "0x0A00" CACHE STRING "Asio-specific Win architecture value (default: Win10)")
  target_compile_definitions(foxy PUBLIC "_WIN32_WINNT=${FOXY_WIN_ARCH}")

  target_compile_options(foxy PUBLIC "/bigobj")

  # keep this around, just in case
  target_link_libraries(foxy PUBLIC Boost::disable_autolinking)
endif()

set(FOXY_QUIET ON CACHE BOOL "Suppresses messages internal to Beast/Asio about deprecated features")
if(FOXY_QUIET)
  target_compile_definitions(
    foxy

    PUBLIC
      BOOST_COROUTINES_NO_DEPRECATION_WARNING=1
      BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE=1
      BOOST_BEAST_ALLOW_DEPRECATED=1
  )
endif()

set(FOXY_FAST_BUILD ON CACHE BOOL "Builds Foxy with separately compiled Beast and Asio")
if(FOXY_FAST_BUILD)
  target_compile_definitions(
    foxy

    PUBLIC
      BOOST_ASIO_SEPARATE_COMPILATION=1
      BOOST_BEAST_SEPARATE_COMPILATION=1
  )

  target_sources(
    foxy

    PRIVATE
      src/asio.cpp
      src/beast.cpp
  )
endif()

target_include_directories(
  foxy

  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/include>
)

target_link_libraries(
  foxy

  PUBLIC
    Boost::boost
    Boost::system
    Boost::date_time
    OpenSSL::SSL
    Threads::Threads
)

if (BUILD_TESTING)
  include(CTest)

  find_package(Catch2 CONFIG REQUIRED)
  find_package(
    Boost 1.70
    CONFIG REQUIRED
      coroutine
      thread
      container
  )

  add_library(test_utils INTERFACE)
  target_include_directories(
    test_utils
    INTERFACE
      ${CMAKE_CURRENT_SOURCE_DIR}/test/include
  )

  add_executable(
    foxy_tests

    test/allocator_client_test.cpp
    test/client_session_test.cpp
    test/export_connect_fields_test.cpp
    test/iterator_test.cpp
    test/main.cpp
    test/pct_decode_test.cpp
    test/pct_encode_test.cpp
    test/proxy_test.cpp
    test/proxy_test2.cpp
    test/relay_test.cpp
    test/server_session_test.cpp
    test/session_test.cpp
    test/ssl_client_session_test.cpp
    test/unicode_uri_test.cpp
    test/uri_parts_test.cpp
    test/uri_test.cpp
    test/utility_test.cpp
  )

  target_link_libraries(
    foxy_tests
    PRIVATE
      foxy
      test_utils
      Catch2::Catch2
      Boost::coroutine
      Boost::thread
      Boost::container
  )

  include(Catch)
  catch_discover_tests(foxy_tests)
endif()

if (FOXY_BUILD_EXAMPLES)
  find_package(
    Boost 1.70
    CONFIG REQUIRED
      coroutine
      thread
      container
  )

  add_executable(
    hello-world

    examples/hello_world/main.cpp
  )

  target_link_libraries(
    hello-world
    PRIVATE
      foxy
      Boost::coroutine
      Boost::thread
  )

  add_executable(
    ip-parser

    examples/uri_combinators/ip_parser.cpp
  )

  target_link_libraries(
    ip-parser
    PRIVATE
      foxy
      Boost::coroutine
      Boost::thread
  )

  add_executable(
    client

    examples/allocators/client.cpp
  )

  target_link_libraries(
    client
    PRIVATE
      foxy
      Boost::container
      Boost::thread
  )
endif()

if (FOXY_FUZZ)
  add_executable(
    uri-parser

    fuzz/parse_uri.cpp
  )

  target_link_libraries(
    uri-parser
    PRIVATE
      foxy
  )

  target_compile_options(
    uri-parser
    PRIVATE
      "-fsanitize=fuzzer"
  )

  target_link_options(
    uri-parser
    PRIVATE
      "-fsanitize=fuzzer"
  )
endif()

# installation
#
install(
  TARGETS
    foxy
  EXPORT
    foxy-targets
  LIBRARY DESTINATION lib/
  ARCHIVE DESTINATION lib/
  RUNTIME DESTINATION bin/
)

install(
  EXPORT
    foxy-targets
  NAMESPACE Foxy::
  DESTINATION lib/cmake/foxy
)

install(
  DIRECTORY
    include

  DESTINATION ${CMAKE_INSTALL_PREFIX}
  FILES_MATCHING PATTERN "*.hpp"
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/foxy-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/foxy-config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/foxy-config.cmake
  INSTALL_DESTINATION lib/cmake/foxy/
)

install(
    FILES
      ${CMAKE_CURRENT_BINARY_DIR}/foxy-config.cmake
      ${CMAKE_CURRENT_BINARY_DIR}/foxy-config-version.cmake
    DESTINATION lib/cmake/foxy/
)
