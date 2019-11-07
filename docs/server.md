# Writing Your Own Server

Foxy provides users a `listener` class that encapsulates the TCP accept loop and connection set-up
and teardown.

---

In this example, we're going to create a localhost server and a local client that'll call out to it.
For the sake of simplicity, we'll only go through one request-response cycle before shutting the
server down.

```c++
#include <foxy/listener.hpp>
#include <foxy/client_session.hpp>
#include <foxy/server_session.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/system/error_code.hpp>

#include <boost/beast/http.hpp>

#include <memory>
#include <iostream>

namespace asio = boost::asio;
namespace http = boost::beast::http;

using boost::asio::ip::tcp;
```

We begin by pulling in all of our required headers. There are quite a few but these can be pulled
into a convenience header for application development. The example chooses explicitness in this
case.

***

```c++
#include <boost/asio/yield.hpp>
```

This header imports Asio's "fauxroutine" pseudo-keyword support. This enables users to `yield` and
`reenter` an [`asio::coroutine`](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/coroutine.html).

***

```c++
struct request_handler : asio::coroutine
{
```

This is the class that's going to implement our server's session with a specific client. In this
case, we're implementing a stackless coroutine to represent this async operation.

***

```c++
  struct frame
  {
    http::request<http::empty_body>   request;
    http::response<http::string_body> response;
  };

  foxy::server_session&  server;
  std::unique_ptr<frame> frame_ptr;
```

Our coroutine (in essence) is nothing more than a simple [Callable](https://en.cppreference.com/w/cpp/named_req/Callable)
and is really more or less a normal callback function. To persist state to our callback, we store a
`unique_ptr` to a "frame" which will contain all the state required. This makes our coroutine
move-only but Asio has support for this.

***

```c++
  request_handler(foxy::server_session& server_)
    : server(server_)
    , frame_ptr(std::make_unique<frame>())
  {
  }
```

Our request handler need only store a reference to the server session as the lifetime is owned by
the `foxy::listener`. We also persist storage for our request and response objects.

***

```c++
  template <class Self>
  auto operator()(Self&                     self,
                  boost::system::error_code ec                = {},
                  std::size_t const         bytes_transferred = 0) -> void
  {
    auto& f = *frame_ptr;
    reenter(*this)
    {
```

This is the start of the body of our coroutine. `reenter` comes from the `yield.hpp` include from
above. Note the usage of `*this`. This is because our server operation inherits from the
`asio::coroutine` class so in this case, we're able to bind `*this` to `asio::coroutine&` which is
required for reentry and setting the suspend point at each `yield`.

***

```c++
      while (true) {
        f.response = {};
        f.request  = {};

        yield server.async_read(f.request, std::move(self));
```

We use a while-loop here to support persistent connections. This means that it's possible for our
server to read in multiple requests from the same client without disconnecting.

For every request-response cycle, we clear out the request/response objects. The `yield` keyword
comes from our `yield.hpp` include and updates the internal `asio::coroutine` to resume the
coroutine after this statement.

Our server session begins by attempting to read in an HTTP request from the underlying stream.
`async_read` will invoke this callable with an error code and the number of bytes transferred.

`self` is our handler that's given to us by the `foxy::listener`. It wraps our `request_handler` so
`std::move`'ing `self` does indeed `move` the `request_handler` itself.

***

```c++
        if (ec) {
          std::cout << "Encountered error when reading in the request!\n" << ec << "\n";
          break;
        }

        std::cout << "Received message!\n" << f.request << "\n";
```

Our coroutine resumes here. If there was an error code, we print out a helpful error message and
let the coroutine end naturally.

***

```c++
        f.response.result(http::status::ok);
        f.response.version(11);
        f.response.body() = "<html><p>Hello, world!</p></html>";
        f.response.prepare_payload();

        yield server.async_write(f.response, std::move(self));

        if (ec) {
          std::cout << "Encountered error when writing the response!\n" << ec << "\n";
          break;
        }
```

This is a "dumb" server in the sense that we don't really process the request but instead send back
a small Hello, World HTML document.

***

```c++
        if (!f.request.keep_alive()) { break; }
      }
```

If the request has keep-alive semantics, repeat the loop over again. For HTTP/1.1, keep-alive is
assumed unless the client sends a `Connection: close` header. In HTTP/1.0, `Connection: keep-alive`
must be set by the client.

This marks the end of our while-loop.

***

```c++
      return self.complete({}, 0);
    }
  }
};
```

This is how we signal to the `foxy::listener` that our server has finished doing what it needed to
and that it should shutdown the session.

***

```c++
#include <boost/asio/unyield.hpp>
```

This undoes the macros that create the pseudo-keywords. This is necessary for proper macro hygiene.

***

```c++
int
main()
{
  asio::io_context io{1};
```

The `1` here is a concurrency hint to the `io_context` about how many threads we'll be running.
Providing this knowledge to the I/O context can enable internal optimizations.

***

```c++
  auto s = foxy::listener(io.get_executor(), tcp::endpoint(asio::ip::make_address("127.0.0.1"),
                                                           static_cast<unsigned short>(1337)));
```

Our server is going to be accessible at: `127.0.0.1:1337`.

***

```c++
  s.async_accept([](auto& server_session) { return request_handler(server_session); });
```

Start the acceptance loop. We no longer touch the listener directly and instead can only call
`shutdown`.

***

```c++
  asio::spawn(io.get_executor(), [&](auto yield) mutable -> void {
```

Our client operation will run on a stackful coroutine (aka a "fiber").

***

```c++
    auto client = foxy::client_session(io.get_executor(),
                                       foxy::session_opts{{}, std::chrono::seconds(30), false});

    client.async_connect("127.0.0.1", "1337", yield);

    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);
    request.keep_alive(false);

    auto response = http::response<http::string_body>();

    client.async_request(request, response, yield);

    std::cout << "Got response back from server!\n" << response << "\n";

    client.stream.plain().shutdown(tcp::socket::shutdown_both);
    client.stream.plain().close();

    s.shutdown();
  });
  });
```

Once our client has connected to our localhost server, sent the request and read the response, we
shutdown our side of the TCP connection and also request that the server close its accept loop.

***

```c++
  io.run();

  return 0;
}
```

Only one thread will run our `io_context`. In this case, the main thread.

Full source code found [here](../examples/server/main.cpp).

---

To [ToC](./index.md#Table-of-Contents)
