set(Boost_DIR "/Users/cmaza/cpp/boost-175/lib/cmake/Boost-1.75.0")
set(FOXY_BUILD_EXAMPLES ON)
set(BUILD_TESTING ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_compile_options("/permissive-")
add_link_options("/NODEFAULTLIB:library" "/debug:fastlink")
add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")

include("/Users/cmaza/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake")
