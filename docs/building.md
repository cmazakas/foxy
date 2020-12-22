# Building

Foxy has only a few non-trivial dependencies. Namely, [Boost](https://github.com/boostorg/boost) and
OpenSSL.

These dependencies can be automatically built using the 
[tools/cmake-superbuild](../tools/cmake-superbuild/CMakelists.txt) project. This 
project simply builds the dependencies and installs them to a local build folder 
before building foxy. You just need to install a recent toolchain and cmake then run:
```bash
cmake -S tools/cmake-superbuild -B build 
cmake --build build
```

Alternatively you can install these dependencies into you host system as 
follows:

## OpenSSL

OpenSSL is found via CMake's [FindOpenSSL](https://cmake.org/cmake/help/latest/module/FindOpenSSL.html)
command.

Most Linux and Unix-like systems will come with OpenSSL installed. For msvc users, the most simple
way of installing OpenSSL is via [Vcpkg](https://github.com/Microsoft/vcpkg).

## Threads

[FindThreads](https://cmake.org/cmake/help/latest/module/FindThreads.html) is also required to
succeed.

## Boost

`find_package(Boost <version> REQUIRED system date_time)` is required to succeed for Foxy to
build.

Boost can be built directly by downloading the releases from [boost.org](https://www.boost.org/).

However, a source-based installation can be desirable. To do so, it's best to use the git
superproject.

While these commands are Linux-focused, many can be ported directly to the Command Prompt in
Windows. Users can perform easy CLI builds in Windows by using the Visual Studio Developer Command
Prompt which will setup all paths to compilers and other tooling.

```bash
# clone the super project into boost-super in the current directory
> git clone https://github.com/boostorg/boost.git boost-super
> cd boost-super

# the Boost superproject is simply a collection of git submodules
> git submodule update --init --recursive

# currently using boost version 1.75
> git checkout boost-1.75.0

# we use the "|| echo 0" to have our foreach loop keep going even in the case of failure
# this can happen from time to time if the submodule list contains a submodule that doesn't have
# the appropriate tag yet
> git submodule foreach "git checkout boost-1.75.0 || echo 0"

# this builds b2, the Boost build tool
Linux:~/$ ./bootstrap.sh
PS> bootstrap

# we then use b2 to complete installation of Boost to the directory denoted by the --prefix option
Linux:~/$ ./b2 install cxxstd=14 --prefix=.../install/directory
PS> b2 install cxxstd=14 address-model=64 --prefix=...\install\directory
```

## Foxy

Building Foxy is a bit more straight-forward:

```bash
> git clone https://github.com/LeonineKing1199/foxy.git
> cd foxy
> mkdir install
> mkdir build
> cd build
> cmake -G"Ninja" -DCMAKE_INSTALL_PREFIX=../install/directory -DCMAKE_TOOLCHAIN_FILE=.../user-toolchain.cmake ..
> cmake --build . --target install
```

This will install Foxy to the directory specified by the `CMAKE_INSTALL_PREFIX`. Foxy can then be
consumed via CMake's `find_package` command provided that the `CMAKE_PREFIX_PATH` is updated to
include the directory that contains Foxy's `foxy-config*.cmake` and `foxy-targets*.cmake` files.

The `user-toolchain.cmake` file should contain something like:

```cmake
# toggle to ON to build examples
set(FOXY_BUILD_EXAMPLES OFF)

set(Boost_USE_STATIC_LIBS ON)

# this contains the make BoostConfig.cmake file
# users should set this appropriately for their system
set(Boost_DIR "/home/chris/boost-spirit-develop/lib/cmake/Boost-1.75.0")

# toggle to ON build tests
set(BUILD_TESTING OFF)

set(CMAKE_CXX_COMPILER "g++-7")

set(CMAKE_BUILD_TYPE Release)
set(BUILD_SHARED_LIBS OFF)

# Foxy's binaries can be quite large because of repetitious template instantiations
# LTO is a wise choice to help slim down the binaries
add_compile_options("-pthread" "-no-pie" "-flto")
add_link_options("-pthread" "-no-pie" "-flto")

add_compile_definitions("BOOST_ASIO_NO_DEPRECATED")

# this is how we'll add OpenSSL to our configuration
include("/home/chris/vcpkg/scripts/buildsystems/vcpkg.cmake")
```

---

To [ToC](./index.md#Table-of-Contents)
