cmake_minimum_required(VERSION 3.0)
project(foxy-super)
include(ExternalProject)

set(DEVROOT ${CMAKE_BINARY_DIR}/devroot)

set(CMAKE_ARGS
        -DCMAKE_BUILD_TYPE=Debug
        -DCMAKE_PREFIX_PATH=${DEVROOT}
        -DCMAKE_INSTALL_PREFIX=${DEVROOT}
        )

ExternalProject_Add(
        boost
	URL https://dl.bintray.com/boostorg/release/1.75.0/source/boost_1_75_0.tar.bz2
	URL_HASH SHA256=953db31e016db7bb207f11432bef7df100516eeb746843fa0486a222e3fd49cb
        BUILD_IN_SOURCE 1
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ./bootstrap.sh --prefix=${DEVROOT}
        BUILD_COMMAND ""
        INSTALL_COMMAND ./b2 --prefix=${DEVROOT}
                             --with-atomic 
                             --with-date_time 
                             --with-system 
                             --with-coroutine 
                             --with-thread 
                             --with-container 
                             --j$(nproc) install
)

ExternalProject_Add(
        catch2
        URL https://github.com/catchorg/Catch2/archive/v2.13.3.tar.gz
        URL_HASH SHA256=fedc5b008f7eb574f45098e7c7138211c543f0f8ad04792090e790511697a877
        CMAKE_ARGS ${CMAKE_ARGS}
            -DCATCH_BUILD_TESTING=OFF
            -DCATCH_ENABLE_WERROR=OFF
            -DCATCH_INSTALL_DOCS=OFF
)

ExternalProject_Add(
        openssl 
        URL https://www.openssl.org/source/openssl-1.1.1i.tar.gz
	URL_HASH SHA1=eb684ba4ed31fe2c48062aead75233ecd36882a6
	CONFIGURE_COMMAND <SOURCE_DIR>/config --prefix=${DEVROOT}
	BUILD_COMMAND make -j1
	INSTALL_COMMAND make install 
)

ExternalProject_Add(
        foxy
        DEPENDS catch2 boost openssl
        SOURCE_DIR ..
        CMAKE_ARGS ${CMAKE_ARGS} -DBUILD_TESTING=ON
)
