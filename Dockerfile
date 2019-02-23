FROM ubuntu:bionic

RUN set -x && \
    apt-get update && \
    apt-get install software-properties-common -y && \
    add-apt-repository ppa:ubuntu-toolchain-r/test -y && \
    apt-get update && \
    apt-get install -y gcc gcc-7 g++-7 wget \
                        git curl unzip tar cmake

ENV CC=gcc-7
ENV CXX=g++-7

RUN set -x && \
    wget -O /tmp/cmake.tar.gz https://github.com/Kitware/CMake/releases/download/v3.13.4/cmake-3.13.4-Linux-x86_64.tar.gz && \
    tar -xzf /tmp/cmake.tar.gz

RUN git clone https://github.com/Microsoft/vcpkg.git && \
    ./vcpkg/bootstrap-vcpkg.sh

RUN set -x && \
    ls vcpkg && \
    touch vcpkg/triplets/x64-linux-cxx14.cmake && \
    cd vcpkg && \
    echo "set(VCPKG_TARGET_ARCHITECTURE x64)" >> triplets/x64-linux-cxx14.cmake && \
    echo "set(VCPKG_LIBRARY_LINKAGE static)" >> triplets/x64-linux-cxx14.cmake && \
    echo "set(VCPKG_CMAKE_SYSTEM_NAME Linux)" >> triplets/x64-linux-cxx14.cmake && \
    echo "set(VCPKG_CRT_LINKAGE dynamic)" >> triplets/x64-linux-cxx14.cmake && \
    echo "set(VCPKG_CXX_FLAGS \"-std=c++14\")" >> triplets/x64-linux-cxx14.cmake && \
    echo "set(VCPKG_C_FLAGS \"\")" >> triplets/x64-linux-cxx14.cmake && \
    echo "set(CMAKE_C_COMPILER gcc-7)" >> triplets/x64-linux-cxx14.cmake && \
    echo "set(CMAKE_CXX_COMPILER g++-7)" >> triplets/x64-linux-cxx14.cmake && \
    CXX=g++-7 ./vcpkg install zlib:x64-linux-cxx14 && \
    CXX=g++-7 ./vcpkg install boost:x64-linux-cxx14 && \
    CXX=g++-7 ./vcpkg install catch2:x64-linux-cxx14

RUN set -x && ls /vcpkg/installed

ENTRYPOINT /bin/bash
