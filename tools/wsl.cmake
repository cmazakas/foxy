set(Boost_DIR "/home/exbigboss/boosts/75/lib/cmake/Boost-1.75.0")

set(BUILD_TESTING ON)
set(Boost_USE_STATIC_LIBS ON)

set(BUILD_SHARED_LIBS OFF)
set(FOXY_BUILD_EXAMPLES ON)
set(FOXY_FUZZ ON)

add_compile_options("-fsanitize=undefined,address")
add_link_options("-fsanitize=undefined,address")

include("/home/exbigboss/vcpkg/scripts/buildsystems/vcpkg.cmake")
