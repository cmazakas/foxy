set(CMAKE_CXX_COMPILER /usr/bin/clang++-8)

set(BUILD_TESTING ON)
set(FOXY_BUILD_EXAMPLES ON)
set(FOXY_FUZZ ON)

add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")

add_compile_options("-fsanitize=address,undefined")
add_link_options("-fsanitize=address,undefined")

# add_compile_options("-fsanitize=thread")
# add_link_options("-fsanitize=thread")

set(Boost_USE_STATIC_LIBS ON)
set(Boost_DIR "/home/chris/boost-175/install/lib/cmake/Boost-1.75.0")

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_SHARED_LIBS OFF)

add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")

include("/home/chris/vcpkg/scripts/buildsystems/vcpkg.cmake")
