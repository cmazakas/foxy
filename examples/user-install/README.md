This is a sample project that demonstrates how to use Foxy for one's own application.

Users are intended to use the CMake files in `tools/` as an example of what a toolchain file should
look like when being used to build applications using Foxy. Foxy needs latest Boost, OpenSSL and on
OSX and Linux, Threads.

To build this example, create a toolchain file appropriate for your system and then run the
following commands:

```bash
cd .../examples/user-install
mkdir build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../tools/my-custom-toolchain.cmake ..
cmake --build .
```

The purpose of this example is to prove that `find_package` will work with Foxy and to also help
guide new users into how they might create a working setup for their system.

The example assumes that a user has created an `install/` directory in the project's root folder.
