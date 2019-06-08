## `foxy::server_session`

## Include

```c++
#include <foxy/server_session.hpp>
```

## Synopsis

The `server_session` functions more as a strong typedef of the `foxy::session` than it does an
independent type. It is intended to represent the session a server has with a client.

Currently not tested with TLS.

Users will have to manually disconnect and close their connections.

For plain connections, users are advised to do something like:
```c++
// http rfc 7230 section 6.6 Tear-down
// -----------------------------------
// To avoid the TCP reset problem, servers typically close a connection
// in stages.  First, the server performs a half-close by closing only
// the write side of the read/write connection.  The server then
// continues to read from the connection until it receives a
// corresponding close by the client, or until the server is reasonably
// certain that its own TCP stack has received the client's
// acknowledgement of the packet(s) containing the server's last
// response.  Finally, the server fully closes the connection.
//
s.session.stream.plain().shutdown(tcp::socket::shutdown_send, ec);

BOOST_ASIO_CORO_YIELD
s.session.async_read(s.shutdown_parser, std::move(*this));

if (ec && ec != http::error::end_of_stream) {
  foxy::log_error(ec, "foxy::proxy::tunnel::shutdown_wait_for_eof_error");
}

s.session.stream.plain().shutdown(tcp::socket::shutdown_receive, ec);
s.session.stream.plain().close(ec);
```

For more information on these methods, see the corresponding documentation for the
[`boost::asio::ip::tcp::socket`](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ip__tcp/socket.html)
and the [`boost::asio::ssl::stream`](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ssl__stream.html)

## Declaration

```c++
struct server_session : public session;
```

## Member Typedefs

```c++
using stream_type   = ::foxy::multi_stream;
using buffer_type   = boost::beast::flat_buffer;
using timer_type    = boost::asio::steady_timer;
using executor_type = typename stream_type::executor_type;
```

## Public Members

```c++
session_opts opts;
stream_type  stream;
buffer_type  buffer;
timer_type   timer;
```

## Constructors

#### Defaults

```c++
server_session()                      = delete;
server_session(server_session const&) = delete;
server_session(server_session&&)      = default;
```

#### `stream`

```c++
explicit server_session(multi_stream stream_);
```

Construct the server session by moving the supplied `multi_stream`.

## Member Functions

#### get_executor

```c++
auto
get_executor() -> executor_type;
```

Return a copy of the underlying executor. Serves as an executor hook.

#### async_read_header

```c++
template <class Parser, class ReadHandler>
auto
async_read_header(Parser& parser, ReadHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                      void(boost::system::error_code, std::size_t)>::return_type;
```

A version of [`boost::beast::http::async_read_header`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__async_read_header.html)
that supports timeouts.

Users can pass either a [`boost::beast::http::message`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__message.html)
or a [`boost::beast::http::parser`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__parser.html)
as the `Parser` template parameter.

The `handler` must be an invocable with a signature of:
```c++
void(boost::system::error_code, std::size_t)
```

The `std::size_t` supplied to the handler is the total number of bytes read from the underlying
stream.

This function will timeout.

#### async_read

```c++
template <class Parser, class ReadHandler>
auto
async_read(Parser& parser, ReadHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                      void(boost::system::error_code, std::size_t)>::return_type;
```

A version of [`boost::beast::http::async_read`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__async_read.html)
that supports timeouts.

Users can pass either a [`boost::beast::http::message`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__message.html)
or a [`boost::beast::http::parser`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__parser.html)
as the `Parser` template parameter.

The `handler` must be an invocable with a signature of:
```c++
void(boost::system::error_code, std::size_t)
```

The `std::size_t` supplied to the handler is the total number of bytes read from the underlying
stream.

This function will timeout.

#### async_write_header

```c++
template <class Serializer, class WriteHandler>
auto
async_write_header(Serializer& serializer, WriteHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<WriteHandler>,
                                      void(boost::system::error_code, std::size_t)>::return_type;
```

A version of [`boost::beast::http::async_write_header`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__async_write_header.html)
that supports timeouts.

Users can pass either a [`boost::beast::http::message`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__message.html)
or a [`boost::beast::http::serializer`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__serializer.html)
as the `Serializer` template parameter.

The `handler` must be an invocable with a signature of:
```c++
void(boost::system::error_code, std::size_t)
```

The `std::size_t` supplied to the handler is the total number of bytes written to the underlying
stream.

This function will timeout.

#### async_write

```c++
template <class Serializer, class WriteHandler>
auto
async_write(Serializer& serializer, WriteHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<WriteHandler>,
                                      void(boost::system::error_code, std::size_t)>::return_type;
```

A version of [`boost::beast::http::async_write`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__async_write.html)
that supports timeouts.

Users can pass either a [`boost::beast::http::message`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__message.html)
or a [`boost::beast::http::serializer`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__http__serializer.html)
as the `Serializer` template parameter.

The `handler` must be an invocable with a signature of:
```c++
void(boost::system::error_code, std::size_t)
```

The `std::size_t` supplied to the handler is the total number of bytes written to the underlying
stream.

This function will timeout.

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
