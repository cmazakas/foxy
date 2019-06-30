## Table of Contents

* [Reference](./reference.md#Reference)
* [Hello World](./hello-world.md#HelloWorld)
* [URI Combinators](./uri-combinators.md#URICombinators)
* [Allocator-Aware GET Request](./allocator-client.md#AllocatorAwareClient)

## About

Foxy is a C++14 library that aims to make idiomatic usage of
[Boost.Beast](https://www.boost.org/doc/libs/release/libs/beast/doc/html/index.html)
and [Boost.Asio](https://www.boost.org/doc/libs/release/doc/html/boost_asio.html)
easier.

Foxy features:
* session-based wrappers for Beast's `http::async_*` functions
* automatic timeouts for stream operations
* transparent support for SSL/TLS
* URI parser combinators (writen in
[Boost.Spirit.X3](https://www.boost.org/doc/libs/release/libs/spirit/doc/x3/html/index.html))
* support for Asio's universal async model of execution
* simple and experimental TLS forward proxy
* pct-encoding for Unicode URLs

## Motivation

Foxy aims to be competitive with the HTTP libraries offered by both Node.js and Go.

While these languages are significantly higher-level than C++ is, their standard HTTP libraries are
low-level from an abstract perspective. The success of these languages and their libraries has shown
that the modern web development is favoring an ever-lower set of HTTP APIs.

Foxy offers a low-level set of session-based primitives that empower users to create their own
abstractions while also offering much more familiar APIs.
