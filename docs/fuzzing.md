# Fuzzing

The `foxy::parse_uri` function has been fuzzed using [libFuzzer](http://llvm.org/docs/LibFuzzer.html)
on both OS X and Linux using clang-6 on OS X and clang-8 on an Ubuntu VM and the Windows Subsystem
for Linux.

The corresponding toolchain files, `tools/foxy-linux-clang.cmake`, `tools/foxy-osx.cmake`,
`tools/wsl.cmake`, contain the compiler flags that show how the fuzzing binaries were built.

A recommeneded dictionary was provided by libFuzzer and is provided in the `fuzz/dict.txt` file.

Fuzzing is accomplished by directly parsing the `char*` and `size_t` pair provided to the target. To
test X3's Unicode handling schemes, we `memcpy` the input bytes to an array object of type
`char32_t`. Currently, X3 will exhibit UB for an invalid Unicode literal. We perform light
sanitization over the Unicode literals array by removing invalid code points:

The fuzz targets are all built with `ubsan` and `asan` enabled and no failures were reported by the
fuzzer.

---

To [ToC](./index.md#Table-of-Contents)
