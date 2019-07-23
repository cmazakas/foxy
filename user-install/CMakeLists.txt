cmake_minimum_required(VERSION 3.10)

project(
  user-example

  LANGUAGES
    CXX
)

find_package(
  Foxy 1.0.0
  CONFIG REQUIRED
)

find_package(
  Boost 1.70
  CONFIG REQUIRED
    system
    date_time
)

find_package(OpenSSL REQUIRED)

add_executable(
  user-example

  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
)

target_link_libraries(
  user-example

  PUBLIC
    Foxy::foxy
    Boost::boost
    Boost::system
    Boost::date_time
    OpenSSL::SSL
)
