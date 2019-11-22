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
  REQUIRED
    system
    date_time
)

find_package(OpenSSL REQUIRED)

if (UNIX)
  set(THREADS_PREFER_PTHREAD_FLAG ON CACHE BOOL "")
endif()

find_package(Threads REQUIRED)

add_library(foxy)
add_subdirectory(include)
add_subdirectory(src)

target_compile_features(foxy PUBLIC cxx_std_14)

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

target_link_libraries(
  foxy

  PUBLIC
    Boost::boost
    Boost::system
    Boost::date_time
    OpenSSL::SSL
    Threads::Threads
)

add_library(
  test_utils

  test/include/foxy/test/helpers/ssl_ctx.hpp
  test/include/foxy/test/helpers/ssl_ctx.cpp
)

target_include_directories(test_utils PUBLIC test/include)
target_link_libraries(test_utils PUBLIC foxy)

include(CTest)
if (BUILD_TESTING)
  add_subdirectory(test)
endif()

if (FOXY_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()

if (FOXY_FUZZ)
  add_subdirectory(fuzz)
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
