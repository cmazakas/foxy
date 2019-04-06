set(Boost_DIR "/Users/cmaza/cpp/boost-170/lib/cmake/Boost-1.70.0")

add_compile_options("/permissive-")
add_link_options("/NODEFAULTLIB:library" "/debug:fastlink")
add_compile_definitions("BOOST_ASIO_NO_DEPRECATED" "_CRT_SECURE_NO_WARNINGS")
include("/Users/cmaza/cpp/vcpkg/scripts/buildsystems/vcpkg.cmake")
