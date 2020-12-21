# Foxy [![Build Status](https://travis-ci.org/LeonineKing1199/foxy.svg?branch=develop)](https://travis-ci.org/LeonineKing1199/foxy)

Low-level HTTP session primitives for Beast/Asio + URL parsing and pct-coding

[Examples and Reference](./docs/index.md#table-of-contents)

## Requirements

* C++14 and above
* CMake 3.13+
* Boost 1.75+
* OpenSSL
* Catch2 (for testing)

[vcpkg](https://github.com/Microsoft/vcpkg) is recommended for easy dependency management.

## Supported Compilers

GCC 7+, Clang 6+, MSVC 2019

## Why Foxy?

* You're a Beast user who wants a reliable HTTP session vocabulary type
* You want a client session abstraction that automatically handles DNS, TLS and certificate verification
* You want a server session abstraction that can detect TLS handshakes and perform them
* You need a URL parser
* You need percent encoding and decoding
* You need a TLS forward proxy
* You want a convenient HTTP(S) server abstraction that handles connection setup + teardown

## Description

Foxy is a C++14 library that aims to make idiomatic usage of
[Boost.Beast](https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/index.html) and
[Boost.Asio](https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio.html)
easier.

Foxy offers users low-level HTTP session primitives. These come in 3 forms:
[session](./docs/reference/session.md#foxybasic_session),
[client_session](./docs/reference/client_session.md#foxybasic_client_session) and
[server_session](./docs/reference/server_session.md#foxybasic_server_session).

The `session` class is direction-agnostic while the `client_session` and `server_session` offer
additional functionality that implementors of clients and servers may find useful.

The utility of the session abstractions is that they abstract away the typical boilerplate required
for using [Boost.Beast](https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/index.html).
Sessions encapsulate sockets, buffers and timers which are all used during stream operations. They
also reduce the API surface and enable implementors to focus solely on using Beast's `message` class
and Asio's executor model.

Foxy also touts a best-in-class [URL parser](./docs/reference/parse_uri.md#foxyparse_uri)
along with a set of URI parsing combinators adopted from
[RFC 3996](https://tools.ietf.org/html/rfc3986#appendix-A). In addition, there are routines for
percent encoding and dedoding URL components.

Foxy is built on top of
[Boost.Beast](https://www.boost.org/doc/libs/1_75_0/libs/beast/doc/html/index.html)
and [Boost.Asio](https://www.boost.org/doc/libs/1_75_0/doc/html/boost_asio.html)
along with Boost.Spirit's library, [X3](https://www.boost.org/doc/libs/1_75_0/libs/spirit/doc/x3/html/index.html).
Foxy does not treat these libraries as implementation details but exposes them directly. To this
end, Foxy is as powerful as plain Beast/Asio are and anything one can do in Beast, one can do using
Foxy.

Foxy aims to be competitive with the HTTP libraries offered by both Node.js and Go.

While these languages are significantly higher-level than C++ is, their standard HTTP libraries are
low-level from an abstract perspective. The success of these languages and their libraries has shown
that the modern web development is favoring an ever-lower set of HTTP APIs.
