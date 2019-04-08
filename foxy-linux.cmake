set(Boost_USE_STATIC_LIBS ON)
set(Boost_DIR "/home/chris/boosts/install/170/lib/cmake/Boost-1.70.0")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_CXX_COMPILER "g++-7")
set(CMAKE_BUILD_TYPE Debug)
set(FOXY_BUILD_EXAMPLES ON)

add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")
include("/home/chris/vcpkg/scripts/buildsystems/vcpkg.cmake")

