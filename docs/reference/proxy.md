## `foxy::proxy`

## Include

```c++
#include <foxy/proxy.hpp>
```

## Synopsis

The `foxy::proxy` is a simple TLS forward proxy, adhering to the standards set forth by RFC 7230 and
7231.

It is intended to take unencrypted local network HTTP and then handle all encryption for the user.
This is particularly useful in supporting legacy applications that will oftentimes have
TLS-deficient HTTP clients.

It also functions in a plain HTTP mode as well so TLS/SSL is not strictly required but it is
recommended.

## Declaration

```c++
struct proxy : public std::enable_shared_from_this<proxy>;
```

## Member Typedefs

```c++
using acceptor_type = boost::asio::ip::tcp::acceptor;
using endpoint_type = boost::asio::ip::tcp::endpoint;
using stream_type   = multi_stream;
using executor_type = boost::asio::strand<typename stream_type::executor_type>;
```

## Constructors

#### Defaults

```c++
proxy()             = delete;
proxy(proxy const&) = delete;
proxy(proxy&&)      = default;
```

#### Parameterized

```c++
proxy(boost::asio::io_context& io,
      endpoint_type const&     endpoint,
      bool                     reuse_addr  = false,
      session_opts             client_opts = {});
```

Construct the proxy's underlying sockets using the provided `io_context` and bind the server to the
user-supplied `endpoint`. `reuse_addr` determines whether or not the TCP acceptor will be
constructed with address re-use enabled like a normal `asio::tcp::acceptor`.

The supplied `client_opts` will be forward to the constructor of the proxy's internal client
session.

## Member Functions

```c++
auto
get_executor() -> executor_type;
```

Return a copy of the proxy's executor type.

```c++
auto
async_accept() -> void;
```

Begins the acceptance loop without blocking the calling thread.

While the acceptance loop will happen on the proxy's internal strand, this is not a thread-safe
operation as it touches the internal `acceptor_type` to check if it's open.

This function is intended to be called once.

```c++
auto
cancel() -> void;
```

Submit a cancellation to the server to terminate the acceptance loop. Dispatches to the proxy's
internal strand so this is a thread-safe operation.

This function will throw if cancellation on the underlying socket throws.

This function is intended to be called once and only after the user has invoked `async_accept`.

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
