# Foxy

[![Build Status](https://travis-ci.org/LeonineKing1199/foxy.svg?branch=master)](https://travis-ci.org/LeonineKing1199/foxy)

Session-based Beast/Asio wrapper

## Requirements

* C++14
* CMake 3.12 and above
* Latest Boost (currently 1.68)
* OpenSSL
* Catch2 (for testing)

We recommend [vcpkg](https://github.com/Microsoft/vcpkg) for dependency
management.

## Supported Compilers

Latest `msvc`, `gcc-7` (Linux), `clang-6` (OS X)

## High-Level Overview

Foxy is built on top of Boost.Beast and Boost.Asio and requires some degree of
familiarity for it to be used successfully. Foxy offers higher-level primitives
than these libraries but still leans into the boons of each library's
abstractions.

Asio handles the raw TCP/IP portions of the application while Beast sits on top
as the HTTP layer. Foxy steps in as a layer on top of Beast/Asio and simply
makes idiomatic usage of Beast and Asio easier.

Beast chooses to treat HTTP as the passing of `message`s. Foxy then makes it
easier to facilitate a _conversation_ between a server and a client. At its
core, a conversation is nothing more than an exchange of messages between two or
more parties.

We choose to call this conversation a "session" or in this case,
`basic_session<Stream>`. The `basic_session` is a class that offers up a
high-level object-oriented interface to Beast's 4 core HTTP functions:
`http::async_read`, `http::async_read_header`, `http::async_write`,
`http::async_write_header`.

We then offer up two higher-level primitives, a `client_session` and a
`server_session`.  The `server_session` acts more like a strong typedef than it
does anything else but the `client_session` offers up client-friendly member
functions: `async_connect` and `async_request`. See the corresponding
`client_session.hpp` header file for documentation.

While Foxy will happily handle establishing connections, it does little in the
way of closing them. Users are expected to manually go through normal Asio
semantics to properly teardown connections. See the Beast and Asio examples for
how this should be done.

## Features

* timeouts for all `session` methods (`async_read`, `async_read_header`, ...)
* transparent support for encrypted/non-encrypted streams
* supports Asio's universal async model

## Example Usage

```cpp
asio::io_context io;

auto session_handle =
  boost::make_unique<foxy::client_session>(io);

auto& session = *session_handle;

// manually readjust the specified timeout for each asynchronous operation on
// the session
//
session.opts.timeout = std::chrono::seconds{10};

auto valid_request = false;

session.async_connect(
  "www.google.com", "http",
  [&valid_request, &session, sh = std::move(session_handle)]
  (error_code ec, tcp::endpoint) mutable -> void
  {
    if (ec) {
      session
        .stream
        .plain()
        .shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      return;
    }

    auto parser_handle  = boost::make_unique<http::response_parser<http::string_body>>();
    auto request_handle =
      boost::make_unique<http::request<http::empty_body>>(http::verb::get, "/", 11);

    auto& parser  = *parser_handle;
    auto& request = *request_handle;

    session.async_request(
      request, parser,
      [
        &valid_request, &session, &parser, &request,
        ph = std::move(parser_handle),
        rh = std::move(request_handle),
        sh = std::move(sh)
      ](error_code ec) -> void
      {
        auto response = parser.release();

        auto is_valid_status = (response.result_int() == 200);
        auto is_valid_body   = (response.body().size() > 0);

        valid_request = is_valid_body && is_valid_status;

        session
          .stream
          .plain()
          .shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      });
  });

io.run();
REQUIRE(valid_request);
```

## Current Status and Roadmap

Project is in its infantile settings but the `client_session` and
`server_session` classes are usable.

Initial minimum viable product is a proxy-aware HTTP client and a small proxy
server implementation.

