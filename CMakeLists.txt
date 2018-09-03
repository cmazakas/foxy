cmake_minimum_required(VERSION 3.12)

project(foxy LANGUAGES CXX)

find_package(
  Boost 1.68
  REQUIRED
    system
    date_time
)

find_package(OpenSSL REQUIRED)

add_library(
  foxy

  STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/client_session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/multi_stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/proxy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/log.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/server_session.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utility.cpp
)

if (MSVC)
  # Win10
  target_compile_definitions(foxy PUBLIC _WIN32_WINNT=0x0A00)
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
    Boost 1.68
    REQUIRED
      coroutine
  )

  add_library(test_utils INTERFACE)
  target_include_directories(
    test_utils
    INTERFACE
      ${CMAKE_CURRENT_SOURCE_DIR}/test/include
  )

  add_executable(
    foxy_tests

    ${CMAKE_CURRENT_SOURCE_DIR}/test/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/client_session_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/proxy_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/utility_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test/export_non_connect_fields.cpp
  )

  if (MSVC)
    target_compile_options(foxy_tests PRIVATE "/bigobj")
  endif()

  target_link_libraries(
    foxy_tests
    PRIVATE
      foxy
      test_utils
      Catch2::Catch2
      Boost::coroutine
  )

  enable_testing()
  include(ParseAndAddCatchTests)
  ParseAndAddCatchTests(foxy_tests)

endif()
