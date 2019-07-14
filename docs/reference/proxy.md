# foxy::proxy

## Include

```c++
#include <foxy/proxy.hpp>
```

## Synopsis

The `foxy::proxy` is a simple TLS forward proxy, adhering to the standards set forth by RFC 7230 and
7231.

It is intended to take unencrypted local network HTTP and then handle all encryption for the user.
This is particularly useful in supporting legacy applications that will oftentimes have
TLS-deficient client-capabilities.

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

### Defaults

```c++
proxy()             = delete;
proxy(proxy const&) = delete;
proxy(proxy&&)      = default;
```

### Parameterized

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

### get_executor

```c++
auto
get_executor() -> executor_type;
```

Return a copy of the proxy's executor type.

### async_accept

```c++
auto
async_accept() -> void;
```

Begins the acceptance loop without blocking the calling thread.

This function is intended to be called once.

### cancel

```c++
auto
cancel() -> void;
```

Submit a cancellation to the server to terminate the acceptance loop. Dispatches to the proxy's
internal strand so this is a thread-safe operation.

This function will throw if cancellation on the underlying socket throws.

This function is intended to be called once and only after the user has invoked `async_accept`.

## Example

This is one of Foxy's tests that demonstrates proper usage of the `foxy::proxy` to forward a client
request over a persistent tunnel.

```c++
using boost::asio::ip::tcp;
namespace ip   = boost::asio::ip;
namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace ssl  = boost::asio::ssl;

using namespace std::chrono_literals;

SECTION("should relay via CONNECT")
{
  asio::io_context io;

  auto was_valid_response = false;

  asio::spawn([&](asio::yield_context yield) {
    auto const addr     = ip::make_address_v4("127.0.0.1");
    auto const port     = static_cast<unsigned short>(1337);
    auto const endpoint = tcp::endpoint(addr, port);

    auto const reuse_addr = true;

    auto proxy = std::make_shared<foxy::proxy>(io, endpoint, reuse_addr);
    proxy->async_accept();

    auto client         = foxy::client_session(io, {});
    client.opts.timeout = 30s;
    client.async_connect("127.0.0.1", "1337", yield);

    auto const request =
          http::request<http::empty_body>(http::verb::connect, "www.google.com:80", 11);

    http::response_parser<http::empty_body> tunnel_parser;
    tunnel_parser.skip(true);

    auto const google_request = http::request<http::empty_body>(http::verb::get, "/", 11);

    http::response_parser<http::string_body> google_parser;

    client.async_request(request, tunnel_parser, yield);
    client.async_request(google_request, google_parser, yield);

    auto ec = boost::system::error_code();
    client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
    client.stream.plain().close(ec);

    auto response = google_parser.release();

    auto const was_valid_result = response.result() == http::status::ok;
    auto const was_valid_body   = response.body().size() > 0;

    was_valid_response = was_valid_result && was_valid_body;
    proxy->cancel();
    proxy.reset();
  });

io.run();
REQUIRE(was_valid_response);
}
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
