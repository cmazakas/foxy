# Intro

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
* simple/experimental TLS forward proxy

## Motivation

Foxy aims to be competitive with the HTTP libraries offered by both Node.js and
Go.

While these languages are significantly higher-level than C++ is, their standard
HTTP libraries are low-level from an abstract perspective. The success of these
languages and their libraries has shown that the modern web development is
favoring an ever-lower set of HTTP APIs.

Beast and Asio are both low-level but in their case, the pendulum has swung too
far in the other direction and they're known for being intimidating and
difficult to use (mainly just Asio).

Foxy steps in and fills this gap, offering a low-level set of session-based
primitives that empower users to create their own abstractions while also
offering much more familiar APIs.
