#
# Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/LeonineKing1199/foxy
#

cmake_minimum_required(VERSION 3.13)

project(foxy LANGUAGES CXX)

find_package(
  Boost 1.69
  REQUIRED
    system
    date_time
)

find_package(OpenSSL REQUIRED)

add_library(
  foxy

  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/detail/close_stream.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/detail/export_connect_fields.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/detail/relay.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/detail/timed_op_wrapper.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/detail/tunnel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/detail/uri_def.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/client_session/async_connect.impl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/client_session/async_request.impl.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/session/async_read_header.impl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/session/async_read.impl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/session/async_write_header.impl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/session/async_write.impl.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/session.impl.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/impl/shared_handler_ptr.impl.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/client_session.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/log.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/multi_stream.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/proxy.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/server_session.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/session.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/shared_handler_ptr.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/type_traits.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/uri_parts.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/uri.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy/utility.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/include/foxy.hpp

  ${CMAKE_CURRENT_SOURCE_DIR}/src/client_session.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/proxy.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/server_session.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/uri_parts.cpp

  # TODO: someday make this work
  #
  # ${CMAKE_CURRENT_SOURCE_DIR}/src/uri.cpp

  ${CMAKE_CURRENT_SOURCE_DIR}/src/utility.cpp
)

if (MSVC)
  # Win10
  target_compile_definitions(foxy PUBLIC _WIN32_WINNT=0x0A00)
  target_compile_options(foxy PUBLIC "/bigobj")
endif()

if (UNIX)
  find_package(Threads REQUIRED)
  target_link_libraries(foxy PUBLIC Threads::Threads)
endif()

target_compile_definitions(
  foxy

  PUBLIC
    BOOST_COROUTINES_NO_DEPRECATION_WARNING=1
    BOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE=1
)

target_compile_features(foxy PUBLIC cxx_std_14)

target_include_directories(
  foxy

  PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(
  foxy

  PUBLIC
    Boost::boost
    Boost::system
    Boost::date_time
    OpenSSL::SSL
)

# keep this around, just in case
if (MSVC)
  target_link_libraries(foxy PUBLIC Boost::disable_autolinking)
endif()

if (FOXY_TESTING)

  find_package(Catch2 CONFIG REQUIRED)
  find_package(
    Boost 1.69
    REQUIRED
      coroutine
      thread
  )

  add_library(test_utils INTERFACE)
  target_include_directories(
    test_utils
    INTERFACE
      ${CMAKE_CURRENT_SOURCE_DIR}/test/include
  )

  add_executable(
    foxy_tests

    ${CMAKE_CURRENT_SOURCE_DIR}/test/client_session_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/export_connect_fields_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/proxy_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/relay_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/session_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/ssl_client_session_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/uri_parts_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/uri_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/utility_test.cpp
  )

  target_link_libraries(
    foxy_tests
    PRIVATE
      foxy
      test_utils
      Catch2::Catch2
      Boost::coroutine
      Boost::thread
  )

  enable_testing()
  include(ParseAndAddCatchTests)
  ParseAndAddCatchTests(foxy_tests)

endif()
