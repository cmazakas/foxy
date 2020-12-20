# foxy::listener

## Include

```c++
#include <foxy/listener.hpp>
```

## Synopsis

The `foxy::listener` acts as a low-level HTTP server that enables users to supply a custom request
handler via a factory function.

The `listener` encapsulates the TCP accept loop and will spawn a new `foxy::server_session` for each
incoming connection. If an `asio::ssl::context` was supplied to the `listener`, the session will
perform its side of the TLS handshake.

Once the connection has been established with the `server_session`, the user's handler factory will
be invoked with a `foxy::server_session&`. Using the returned handler, the `listener` will then
invoke it via `asio::async_compose`.

New connections are given their own strand and the TCP accept loop runs on its own strand. This
means that user's request handlers are given an implicit strand even when the `io_context` is being
run across multiple threads.

The `foxy::listener` handles the creation and teardown of server-based connections so users do not
need to do any direct handling of connection lifetimes themselves.

## Declaration

```c++
struct foxy::listener;
```

## Member Typedefs

```c++
using executor_type = boost::asio::strand<boost::asio::any_io_executor>;
```

## Constructors

### Defaults

```c++
listener()                = delete;
listener(listener const&) = delete;
listener(listener&&)      = default;
```

### Parameterized

```c++
listener(boost::asio::any_io_executor executor, boost::asio::ip::tcp::endpoint endpoint);
```

Create a new `listener` which will use the supplied `executor` for its I/O objects and an `endpoint`
which the `listener` will bind to and listen on during construction.

Callers must _not_ supply an `asio::strand` for the executor. The `foxy::listener` will wrap the
supplied executor in a strand for the caller. Passing in an explicit strand will simply
double-strand the nested executor (not incorrect but not ideal).

```c++
listener(boost::asio::any_io_executor          executor,
         boost::asio::ip::tcp::endpoint endpoint,
         boost::asio::ssl::context      ctx);
```

Create a new `listener` that will perform a TLS/SSL handshake using the supplied `ssl::context`. The
`listener` will _require_ clients perform an SSL handshake.

Callers must _not_ supply an `asio::strand` for the executor. The `foxy::listener` will wrap the
supplied executor in a strand for the caller. Passing in an explicit strand will simply
double-strand the nested executor (not incorrect but not ideal).

## Member Functions

### get_executor

```c++
auto
get_executor() const noexcept -> executor_type;
```

Return a copy of the `listener`'s executor.

### async_accept

```c++
template <class RequestHandlerFactory>
auto
async_accept(RequestHandlerFactory&& factory) -> void;
```

Begin the TCP acceptance loop.

The caller is required to supply a "request handler factory" which is a callable that accepts a
`foxy::server_session&` and returns a type such that `asio::async_compose` is well-formed using the
`factory`'s return as the initiator.

Callables to `async_compose` must accept a mutable lvalue reference to the handler (which Asio
automatically wraps around theirs). The callable must be invocable as: `callable(self)` which
means if a user wants their callable to accept arguments, they need to be defaulted explicitly.

It is worth noting that once a user performs `std::move(self)`, the callable is then in a moved-from
state so all variables embedded in the callable are now moved-from as well.

As an example:

```c++
struct request_handler
{
  foxy::server_session& server;

  template <class Self>
  auto operator()(Self& self, boost::system::error_code ec = {}, std::size_t bytes_transferred = 0)
    -> void
  {
    self.complete({}, 0);
  }
};

auto
request_handler_factory(foxy::server_session& server) -> request_handler
{
  return {server};
}
```

### shutdown

```c++
auto
shutdown() -> void;
```

Submit a cancellation to the TCP acceptor, interrupting its loop and not re-starting it.

Does not block.

All currently active running sessions will continue to run until they naturally close.

## Example

An example can be found [here](../../examples/listener/main.cpp).

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
