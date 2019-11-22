add_executable(uri-parser parse_uri.cpp)
target_link_libraries(uri-parser PRIVATE foxy)
target_compile_options(uri-parser PRIVATE "-fsanitize=fuzzer")
target_link_options(uri-parser PRIVATE "-fsanitize=fuzzer")
