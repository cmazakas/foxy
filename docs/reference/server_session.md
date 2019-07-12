## `foxy::basic_server_session`

## Include

```c++
#include <foxy/server_session.hpp>
```

## Synopsis

The `basic_server_session` encapsulates a small subset of what a server may need to do to handle an
HTTP transaction.

The server session can only be constructed by a living socket which must be moved into the session's
constructor.

This is to have the design follow that of `accept` which returns a new file handle to a socket upon
successfully accepting a connection. The server session is intended to be constructed by
`asio::ip::tcp::acceptor`'s `accept` or `async_accept` methods.

The session enables users to decouple detecting a client request for a TLS upgrade from performing
the actual handshake. Using this, users can support both plain HTTP and HTTPS on the same port as
is the case with the [example server in Beast](https://www.boost.org/doc/libs/release/libs/beast/example/advanced/server-flex/advanced_server_flex.cpp).

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
template <class DynamicBuffer>
struct basic_server_session
  : public basic_session<
      boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                       typename boost::asio::io_context::executor_type>,
      DynamicBuffer>;
```

## Member Typedefs

```c++
using stream_type   = ::foxy::basic_multi_stream<Stream>;
using buffer_type   = DynamicBuffer;
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
basic_server_session()                            = delete;
basic_server_session(basic_server_session const&) = delete;
basic_server_session(basic_server_session&&)      = default;
```

#### `stream`

```c++
template <class... BufferArgs>
basic_server_session(multi_stream stream_, session_opts opts, BufferArgs&&... bargs);
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

#### async_detect_ssl

```c++
template <class DetectHandler>
auto
async_detect_ssl(DetectHandler&& handler) ->
  typename boost::asio::async_result<std::decay_t<DetectHandler>,
                                     void(boost::system::error_code, bool)>::return_type;
```

A version of [`boost::beast::async_detect_ssl`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__async_detect_ssl.html)
that supports timeouts.

This function will read from the socket until it either detects a client SSL handshake, reads in
enough data to know it is not receiving a handshake or the connection is timed out or any other
error occurs while reading from the socket.

This function is intended to be used when the server session is in its plain mode.

The `handler` must be an invocable with a signature of:
```c++
void(boost::system::error_code, bool)
```

The supplied boolean indicates whether or not an SSL handshake was detected.

This function will timeout.

#### async_handshake

```c++
template <class HandshakeHandler>
auto
async_handshake(HandshakeHandler&& handler) ->
  typename boost::asio::async_result<std::decay_t<HandshakeHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type;
```

Performs the server portion of an SSL handshake. Implicitly requires the session's stream to be in
SSL mode, i.e. `session.stream.is_ssl()` returns `true`.

Users are intended to call [`upgrade`](./multi_stream.md#upgrade) on the underlying stream object
with an appropriate `asio::ssl::context` before calling this function.

This function will timeout.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
