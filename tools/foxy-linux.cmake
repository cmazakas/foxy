set(FOXY_BUILD_EXAMPLES ON)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_DIR "/home/chris/boost-175/install/lib/cmake/Boost-1.75.0")

set(BUILD_TESTING ON)
set(CMAKE_CXX_COMPILER "g++-7")
set(CMAKE_BUILD_TYPE Debug)
set(BUILD_SHARED_LIBS OFF)

add_compile_options("-pthread" "-no-pie" "-flto")
add_link_options("-pthread" "-no-pie" "-flto")

# add_compile_options("-fsanitize=undefined,address")
# add_link_options("-fsanitize=undefined,address")

# add_compile_options("-fsanitize=thread")
# add_link_options("-fsanitize=thread")

add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")
include("/home/chris/vcpkg/scripts/buildsystems/vcpkg.cmake")
