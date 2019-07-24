set(BUILD_TESTING ON)
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};/Users/exbigboss/boosts/70-spirit-develop/lib/cmake/Boost-1.71.0")

add_compile_options("-fsanitize=address,undefined")
add_link_options("-fsanitize=address,undefined")

# add_compile_options("-fsanitize=thread")
# add_link_options("-fsanitize=thread")

add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")
include("/Users/exbigboss/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(CMAKE_INSTALL_PREFIX "/Users/exbigboss/cpp/f3/install")
