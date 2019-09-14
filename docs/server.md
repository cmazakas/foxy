# Writing Your Own Server

Foxy doesn't do many things. One of those is provide a universal server abstraction. It's not within
Foxy's immediate scope to provide users with a way of creating a TCP accept loop as there's no one
universal abstraction in C++ that'll satisfy all needs.

This example attempts to demonstrate how one would begin to write an HTTP server using Foxy by
providing readers with a working example of a `server` class they can model their own after.

---

In this example, we're going to create a localhost server and a local client that'll call out to it.
For the sake of simplicity, we'll only go through one request-response cycle before shutting the
server down.

```c++
#include <foxy/client_session.hpp>
#include <foxy/server_session.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

#include <boost/asio/coroutine.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/executor.hpp>

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
struct server_op : asio::coroutine
{
```

This is the class that's going to implement our server's session with a specific client. In this
case, we're implementing a stackless coroutine to represent this async operation.

***

```c++
  using executor_type = asio::executor;
```

Asio needs this typedef to form an "executor hook". Asio will use this typedef to reason about how
this coroutine should be executed. This is important because Asio implicitly relies upon the notion
of a strand (either implicit or explicit) as discussed [here](https://www.boost.org/doc/libs/release/doc/html/boost_asio/overview/core/strands.html).

***

```c++
  struct frame
  {
    foxy::server_session              server;
    http::request<http::empty_body>   request;
    http::response<http::string_body> response;

    frame(tcp::socket socket, foxy::session_opts opts)
      : server(foxy::multi_stream(std::move(socket)), opts)
    {
    }
  };

  std::unique_ptr<frame> frame_ptr;
```

Our coroutine (in essence) is nothing more than a simple [Callable](https://en.cppreference.com/w/cpp/named_req/Callable)
and is really more or less a normal callback function. To persist state to our callback, we store a
`unique_ptr` to a "frame" which will contain all the state required. This makes our coroutine
move-only but Asio has support for this.

***

```c++
  server_op(tcp::socket stream)
    : frame_ptr(std::make_unique<frame>(std::move(stream),
                                        foxy::session_opts{{}, std::chrono::seconds(30), false}))
  {
  }
```

Our `server_session` will need a connected TCP socket along with an options object to be
constructed. In this case, we're constructing our session options with an empty SSL context, a 30
second timeout and we're disabling peer certificate verification (though the server session
currently doesn't use this anyway).

***

```c++
  auto operator()(boost::system::error_code ec = {}, std::size_t const bytes_transferred = 0)
    -> void
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

        yield f.server.async_read(f.request, std::move(*this));
```

We use a while-loop here to support persistent connections. This means that it's possible for our
server to read in multiple requests from the same client without disconnecting.

For every request-response cycle, we clear out the request/response objects. The `yield` keyword
comes from our `yield.hpp` include and updates the internal `asio::coroutine` to resume the
coroutine after this statement.

Our server session begins by attempting to read in an HTTP request from the underlying stream.
`async_read` will invoke this callable with an error code and the number of bytes transferred.

***

```c++
        if (ec) {
          std::cout << "Encountered error when reading in the request!\n" << ec << "\n";
          goto shutdown;
        }

        std::cout << "Received message!\n" << f.request << "\n";
```

Our coroutine resumes here. If there was an error code, we print out a helpful error message and
jump to the shutdown procedure.

***

```c++
        f.response.result(http::status::ok);
        f.response.version(11);
        f.response.body() = "<html><p>Hello, world!</p></html>";
        f.response.prepare_payload();

        yield f.server.async_write(f.response, std::move(*this));

        if (ec) {
          std::cout << "Encountered error when writing the response!\n" << ec << "\n";
          goto shutdown;
        }
```

This is a "dumb" server in the sense that we don't really process the request but instead send back
a small Hello, World HTML document.

***

```c++
        if (f.request.keep_alive()) { continue; }
```

If the request has keep-alive semantics, repeat the loop over again. For HTTP/1.1, keep-alive is
assumed unless the client sends a `Connection: close` header. In HTTP/1.0, `Connection: keep-alive`
must be set by the client.

***

```c++
      shutdown:
        f.server.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
        f.server.stream.plain().close(ec);
        break;
      }
    }
  }
```

Our shutdown procedure is pretty simple. We access the plain TCP side of our session's nested
multi-stream and follow normal Asio TCP shutdown semantics, calling `shutdown` and then `close`.

***

```c++
  auto
  get_executor() const noexcept -> executor_type
  {
    return frame_ptr->server.get_executor();
  }
};
```

This is the other half of the executor hook. Asio will use this member function to grab a copy of
our coroutine's executor and then post the resumption of our coroutine to it.

***

```c++
struct accept_op : asio::coroutine
{
```

Our `accept_op` is relatively similar to our `server_op` above. For our server to successfully
handle multiple clients, we need several concurrent tasks. So we define a coroutine for the actual
session our server will have with a client along with a persistent async operation that will
continue to listen for incoming TCP connections.

***

```c++
  using executor_type = asio::executor;

  struct frame
  {
    tcp::socket socket;

    frame(asio::executor executor)
      : socket(executor)
    {
    }
  };

  tcp::acceptor&         acceptor;
  std::unique_ptr<frame> frame_ptr;

  accept_op(tcp::acceptor& acceptor_)
    : acceptor(acceptor_)
    , frame_ptr(std::make_unique<frame>(acceptor.get_executor()))
  {
  }
```

This time our coroutine will only need to store non-owning view of an `asio::ip::tcp::acceptor` and
a local socket that the `async_accept` method can use to write the connected TCP socket to. This
socket will be used to construct our `server_session` and be re-used during future accept calls.

***

```c++
  auto operator()(boost::system::error_code ec = {}) -> void
  {
    auto& f = *frame_ptr;
    reenter(*this)
    {
      while (acceptor.is_open()) {
        yield acceptor.async_accept(f.socket, std::move(*this));
        if (ec == asio::error::operation_aborted) { yield break; }
        if (ec) {
          std::cout << "Failed to accept the new connection!\n";
          std::cout << ec << "\n";
          yield break;
        }
```

This is the core of a TCP accept loop in Asio. We accept new connections indefinitely, assuming we
don't have some sort of an error beyond our acceptor simply being `close`'d or `cancel`'d.

***

```c++
        asio::post(server_op(std::move(f.socket)));
      }
    }
  }
```

We create our `server_op` and then `asio::post` it for execution. `post` is smart enough to use the
executor hooks on our `server_op` and will run the coroutine on that executor. This is important
for correctness.

***

```c++
  auto
  get_executor() const noexcept -> executor_type
  {
    return acceptor.get_executor();
  }
};

struct server
{
```

Finally! Our actual server class. This will be what user's interact with directly. The above
coroutines operate as implementation details of this interface.

***

```c++
  using executor_type = asio::executor;

  tcp::acceptor acceptor;

  server()              = delete;
  server(server const&) = delete;
  server(server&&)      = default;

  server(executor_type executor, tcp::endpoint endpoint)
    : acceptor(executor, endpoint)
  {
  }

  auto
  get_executor() -> executor_type
  {
    return acceptor.get_executor();
  }

  auto
  async_accept() -> void
  {
    asio::post(accept_op(acceptor));
  }
```

A note about `async_accept` is that it's UB to now modify the acceptor from outside the strand
that's currently running. What this means is, if one is only using a single-thread to run the
`io_context`, they're safe and nothing else needs to be worried about.

But if this was a multi-threaded server, the `accept_op` could begin immediately on a new thread and
while the accept loop would be thread-safe, touching the acceptor from the currently running thread
may result in a race condition unless that thread is already running in the same `asio::strand` that
was given to the server as its executor.

This is a single-threaded example though so the above note doesn't apply but it is worth mentioning.

***

```c++
  auto
  shutdown() -> void
  {
    asio::post(get_executor(), [self = this]() mutable -> void {
      self->acceptor.cancel();
      self->acceptor.close();
    });
  }
};
```

Shutting the server down is a thread-safe function because it copies the executor and then
dispatches a callable that'll run in the executor. In this case, we simply call `cancel()` and
`close()` on the TCP acceptor.

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
  auto const endpoint =
    tcp::endpoint(asio::ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337));
```

Our server is going to be accessible at: `127.0.0.1:1337`.

***

```c++
  auto s = server(io.get_executor(), endpoint);
  s.async_accept();
```

Create the server and start the acceptance loop. We no longer touch the acceptor directly and
instead can only call `shutdown`.

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
