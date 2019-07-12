# Hello World

In many web applications, the canonical "hello, world!" program is represented by a simple client
GET to a well-known URL.

For our purposes, we'll be testing Foxy against `www.google.com`.

But before we just dive in and look at the end code, it's useful to review Foxy's...

## Core Abstractions

Foxy's core abstractions revolve around the notion of an HTTP session. A session represents a
persistent connection between a client and server over which many messages are exchanged. For HTTP,
connections are considered persistent by default under HTTP/1.1 and when the
`connection: keep-alive` header field is used in HTTP/1.0.

Foxy presents users with a `foxy::basic_session` which is a class templated on a `Stream` type.

Any `Stream` can be supplied so long as
```c++
boost::beast::is_async_stream<Stream>::value
```
returns true.

The `foxy::basic_session<Stream>` aims to abstract away the necessary parameters for using
`boost::beast::http::async_read` and `boost::beast::http::async_write`.

The `foxy::basic_session` wraps an instance of a `foxy::basic_multi_stream<Stream>` object, a
`boost::beast::flat_buffer`, a `boost::asio::steady_timer` and a copy of `foxy::session_opts`.

`foxy::session_opts` is a class internally used by the library to control things like timeouts and
SSL.

The `foxy::basic_multi_stream` is a templated class that wraps a given `Stream` in a variant-like
type such that the underlying stream operations are done plainly or over SSL. This enables the same
`async_read_some`/`async_write_some` interface as a normal `AsyncStream` but is transparent with
regards to encryption.

This allows the `foxy::basic_session` to reduce the number of user-supplied parameters to the HTTP
parsing and serialization functions (and also enables the library to internally manage the buffer
for users as well).

One can perform:
```c++
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

## Client Session

Client functionality is not nearly as generic or as simple parsing/serailizing functions. Foxy
offers a `foxy::client_session` which is an HTTP client-oriented class. It offers two additional
functions over the `foxy::basic_session`: `async_connect` and `async_request`. The `client_session`
is _not_ templated on its underlying stream type and instead is hard-coded to use the
`boost::asio::ip::tcp::socket`.

`foxy::client_session::async_connect` is used to connect with a remote and will first perform DNS
resolution, establish the TCP connection and then, if needed, perform the SSL handshake.

`foxy::client_session::async_request` will write the message/serializer to the underlying stream of
the session and then read the response back into the supplied parser/message. It's largely a
convenience function when all you want is a simple request-response cycle.

## "Hello, World!"

Now that most of Foxy's basics have been covered, we can finally dive into the code and make a fully
working Foxy "Hello, World!" client.

```c++
#include <foxy/client_session.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include <iostream>

namespace asio = boost::asio;
namespace http = boost::beast::http;

int
main()
{
  // this is the main workhorse for Asio and effectively operates the same as the event loop in
  // Node.js
  //
  asio::io_context io;

  // we now construct a `client_session` with the default parameters (no SSL, timeout of 1s on
  // read/write ops)
  //
  auto client = foxy::client_session(io, {});

  // we now choose to spin up a "stackful" coroutine
  // this essentially means that this coroutine suspends by taking a snapshot of the current call
  // stack and any registers and persists them to some allocated storage
  //
  // the coroutine resumes by copying the stack back and setting all the appropriate registers again
  //
  // this effectively enables the coroutine to be suspended from below us as the actual suspension
  // happens in our underlying function calls
  //
  asio::spawn([&](asio::yield_context yield) {
    // these arguments get forwarded directly to Asio via Foxy
    // `host` is somewhat self-explanatory but `service` can be either a port number directly or a
    // protocol as is demonstrated here
    //
    auto const* const host    = "www.google.com";
    auto const* const service = "http";

    auto const http_version = 11; // 1.1

    // we build our message containers up-front
    // `request` is a simple "GET / HTTP/1.1\r\n\r\n"
    // `response` is declared up-front as the intended container type for the reply from Google
    //
    auto request  = http::request<http::empty_body>(http::verb::get, "/", http_version);
    auto response = http::response<http::string_body>();

    // we now attempt to connect to google.com, using plain HTTP
    // note, if there is an error, this coroutine will be resumed with an exception
    //
    client.async_connect(host, service, yield);

    // we now write the request and read the response
    //
    client.async_request(request, response, yield);

    // closing a connection is either synchronous or asynchronous depending on whether or not the
    // underlying stream is encrypted or not
    // Foxy doesn't directly handle this so it's up to the user to gracefully close the connection
    // with the connected remote
    //
    // we access the plain portion of the wrapped multi-stream and do normal Asio TCP connection
    // closing
    //
    auto& socket = client.stream.plain();
    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.close();

    std::cout << "Got a response back from Google!\n";
    std::cout << response << "\n\n";

    // at this stage, we've done everything we needed to!
    // we turned the human-readable domain name into a set of IPs, formed a TCP connection with the
    // first one we could and then wrote a request object to the underlying TCP stream and read the
    // response back in a container that we control ourselves
    // we finish off with a bit of manual work, gracefully closing the TCP connection ourselves
    //
  });

  // we've created the code for our coroutine but we haven't actually done any work yet
  // we must now register the main thread as a worker thread for Asio's `io_context`
  // `io_context::run` will keep the calling thread busy until there's no more work to do
  // in this case, we have only one submitted task right now, our above coroutine
  //
  io.run();

  return 0;
}
```

---

To [ToC](./index.md#Table-of-Contents)
