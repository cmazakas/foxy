set(FOXY_TESTING ON)
set(BOOST_ROOT /home/chris/boosts/68/install)
set(Boost_USE_STATIC_LIBS ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_COMPILER "g++-7")
# add_compile_options("-fsanitize=undefined")

include(/home/chris/vcpkg/scripts/buildsystems/vcpkg.cmake)
