set(Boost_DIR "/Users/cmaza/cpp/boost-171/lib/cmake/Boost-1.71.0")
set(FOXY_BUILD_EXAMPLES ON)
set(BUILD_TESTING ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PostgreSQL_INCLUDE_DIR "/Program Files/PostgreSQL/11/include")
set(PostgreSQL_LIBRARY_DIR "/Program Files/PostgreSQL/11/lib")

add_compile_options("/permissive-")
add_link_options("/NODEFAULTLIB:library" "/debug:fastlink")
add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")

include("/Users/cmaza/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake")
