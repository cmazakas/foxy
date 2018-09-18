set(FOXY_TESTING ON)
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_COMPILER "g++-7")
set(VCPKG_TARGET_TRIPLET "x64-linux-cxx14")

include(/home/travis/build/LeonineKing1199/f3/vcpkg/scripts/buildsystems/vcpkg.cmake)
