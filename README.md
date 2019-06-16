# Foxy

[![Build Status](https://travis-ci.org/LeonineKing1199/foxy.svg?branch=master)](https://travis-ci.org/LeonineKing1199/foxy)

Session-based Beast/Asio wrapper + URL parsing and pct-encoding

Read the docs [here](./docs/index.md).

## Requirements

* C++14
* CMake 3.10 and above
* Latest Boost (currently 1.70 [currently requires `develop` version of Spirit])
* OpenSSL
* Catch2 (for testing)

[vcpkg](https://github.com/Microsoft/vcpkg) is recommended for easy dependency management.

## Supported Compilers

Latest `msvc`, `gcc-7`, `clang-6`, `clang-8`

## High-Level Overview

Foxy is built on top of [Beast](https://www.boost.org/doc/libs/release/libs/beast/doc/html/index.html)
and [Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
and requires some degree of familiarity with both of these libraries to be used successfully.

One of Foxy's core tenets is rewarding users for deepening their knowledge and understanding of
Beast and Asio and this is reflected in the design.

Foxy provides primitives for HTTP sessions as well as a rich set of URI parser combinators along
with methods for parsing (and pct-encoding) URLs in both ASCII and Unicode. Foxy bases its Unicode
handling off of `char32_t`. An experimental TLS forward proxy is provided as well.

## Features

* built-in timeouts
* transparent support for encrypted/non-encrypted streams
* supports Asio's universal async model
* TLS forward proxy
* URL parsing and encoding

## Current Status and Roadmap

Project is young but the `client_session` and `server_session` classes are usable.

Initial minimum viable product is a proxy-aware HTTP client and a small proxy server implementation.
