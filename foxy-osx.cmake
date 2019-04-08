set(FOXY_TESTING ON)
# set(Boost_USE_STATIC_LIBS ON)
# set(BOOST_ROOT "/Users/exbigboss/boosts/69/install")

set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/Users/exbigboss/boosts/70/lib/cmake/Boost-1.70.0")

add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")
include("/Users/exbigboss/vcpkg/scripts/buildsystems/vcpkg.cmake")

