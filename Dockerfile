FROM ubuntu:bionic

RUN apt-get update
RUN apt-get install software-properties-common -y
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt-get update
RUN apt-get install gcc -y
RUN apt-get install gcc-7 -y
RUN apt-get install g++-7 -y
RUN apt-get install wget -y
RUN apt-get install git -y
RUN apt-get install curl unzip tar -y
RUN apt-get install cmake -y

RUN export CC=gcc-7
RUN export CXX=g++-7

RUN wget https://cmake.org/files/v3.12/cmake-3.12.2-Linux-x86_64.tar.gz -O /tmp/cmake.tar.gz
RUN tar -xzf /tmp/cmake.tar.gz

RUN git clone https://github.com/Microsoft/vcpkg.git
RUN ./vcpkg/bootstrap-vcpkg.sh

RUN ls vcpkg
RUN touch vcpkg/triplets/x64-linux-cxx14.cmake
RUN cd vcpkg \
&& echo "set(VCPKG_TARGET_ARCHITECTURE x64)" >> triplets/x64-linux-cxx14.cmake \
&& echo "set(VCPKG_LIBRARY_LINKAGE static)" >> triplets/x64-linux-cxx14.cmake \
&& echo "set(VCPKG_CMAKE_SYSTEM_NAME Linux)" >> triplets/x64-linux-cxx14.cmake \
&& echo "set(VCPKG_CRT_LINKAGE dynamic)" >> triplets/x64-linux-cxx14.cmake \
&& echo "set(VCPKG_CXX_FLAGS \"-std=c++14\")" >> triplets/x64-linux-cxx14.cmake \
&& echo "set(VCPKG_C_FLAGS \"\")" >> triplets/x64-linux-cxx14.cmake \
&& echo "set(CMAKE_C_COMPILER gcc-7)" >> triplets/x64-linux-cxx14.cmake \
&& echo "set(CMAKE_CXX_COMPILER g++-7)" >> triplets/x64-linux-cxx14.cmake \
&& CXX=g++-7 ./vcpkg install zlib:x64-linux-cxx14 \
&& CXX=g++-7 ./vcpkg install boost-regex:x64-linux-cxx14 \
&& CXX=g++-7 ./vcpkg install boost-spirit:x64-linux-cxx14 \
&& CXX=g++-7 ./vcpkg install boost-asio:x64-linux-cxx14 \
&& CXX=g++-7 ./vcpkg install boost-beast:x64-linux-cxx14 \
&& CXX=g++-7 ./vcpkg install catch2:x64-linux-cxx14

# RUN git clone https://github.com/LeonineKing1199/f3.git

RUN ls /vcpkg/installed

# RUN cd f3 \
# && touch "foxy-docker.cmake" \
# && echo "set(FOXY_TESTING ON)" >> foxy-docker.cmake \
# && echo "set(CMAKE_BUILD_TYPE Debug)" >> foxy-docker.cmake \
# && echo "set(CMAKE_CXX_COMPILER \"g++-7\")" >> foxy-docker.cmake \
# && echo "include(\"/vcpkg/scripts/buildsystems/vcpkg.cmake\")" >> foxy-docker.cmake

# RUN mkdir f3/build_debug
# RUN cd f3/build_debug \
# && /cmake-3.12.2-Linux-x86_64/bin/cmake \
# -DVCPKG_TARGET_TRIPLET="x64-linux-cxx14" \
# -DCMAKE_TOOLCHAIN_FILE=../foxy-docker.cmake .. \
# && /cmake-3.12.2-Linux-x86_64/bin/cmake --build . --target all

ENTRYPOINT /bin/bash
