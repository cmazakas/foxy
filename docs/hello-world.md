# Hello World

In many web applications, the canonical "hello, world!" program is represented by a simple client
GET to a well-known URL.

For our purposes, we'll be testing Foxy against `www.google.com`.

## Core Abstractions

Foxy's core abstraction revolves around the idea of an HTTP session, where an HTTP "session"
represents several messages passed back and forth between a server and a client over the same
TCP connection.

Foxy presents users with a `foxy::basic_session` which is a class templated on a `Stream` type.

Any `Stream` can be supplied so long as
```
boost::beast::is_async_stream<Stream>::value
```
returns true.

The `foxy::basic_session<Stream>` aims to abstract away the necessary parameters for using
`boost::beast::http::async_read` and `boost::beast::http::async_write`.

The `foxy::basic_session` wraps an instance of a `foxy::basic_multi_stream<Stream>` object, a
`boost::beast::flat_buffer`, a `boost::asio::steady_timer` and a copy of `foxy::session_opts`.
`foxy::session_opts` is a class internally used by the library to control things like timeouts and
SSL.

This allows the `foxy::basic_session` to reduce the number of user-supplied parameters to the HTTP
parsing functions (and also enables the library to internally manage the buffer for users as well).

One can perform:
```
namespace http = boost::beast::http;

foxy::basic_session<Stream> session{io_context};

// read a message in
//
http::request_parser<http::string_body> req_parser;
session.async_read(req_parser, yield);

// write the same message back out
//
auto message = req_parser.release();
session.async_write(message, yield);
```

The user is free to parse/serialize any valid Beast type and Foxy will also respect any
Asio-compliant CompletionHandlers supplied to the functions. Foxy dispatches heavily to Asio and
respects its universal asynchronous model.

The `foxy::basic_multi_stream` is a templated class that wraps a given `Stream` in a variant-like
type such that the underlying stream operations are done plainly or over SSL. This enables the same
`async_read_some`/`async_write_some` interface as a normal `AsyncStream` but is transparent with
regards to encryption.

## Client Session

Client functionality is not nearly as generic as simple parsing/serailizing functions. Foxy offers a
`foxy::client_session` which is an HTTP client-oriented class. It offers two additional functions
over the `foxy::basic_session`: `async_connect` and `async_request`. The `client_session` is _not_
templated on its underlying stream type and instead is hard-coded to use the
`boost::asio::ip::tcp::socket`.



[To ToC](./intro.md#Table-of-Contents)
