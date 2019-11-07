# foxy::basic_server_session

## Include

```c++
#include <foxy/server_session.hpp>
```

## Synopsis

The `basic_server_session` encapsulates a small subset of what a server may need to do to handle an
HTTP transaction.

The server session can only be constructed by a connected socket which must be moved into the
session's constructor.

This is to have the design follow that of [`accept`](http://man7.org/linux/man-pages/man2/accept.2.html)
which returns a new file handle to a socket upon successfully accepting a connection. The server
session is intended to be constructed by `asio::ip::tcp::acceptor`'s `accept` or `async_accept`
methods.

The session also enables users to decouple detecting a client request for a TLS upgrade from
performing the actual handshake. Using this, users can support both plain HTTP and HTTPS on the same
port as is the case with the
[example server in Beast](https://www.boost.org/doc/libs/release/libs/beast/example/advanced/server-flex/advanced_server_flex.cpp).

When constructing the `basic_server_session`, if the supplied `session_opts` contain an
`ssl::context`, it is _not_ used to upgrade the underlying stream. Instead, they are stored as for
later use either by `basic_server_session::async_handshake` or `multi_stream::upgrade` (which the
handshake function calls under the hood anyway).

## Declaration

```c++
template <class DynamicBuffer>
struct basic_server_session : public basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>;
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

### Defaults

```c++
basic_server_session()                            = delete;
basic_server_session(basic_server_session const&) = delete;
basic_server_session(basic_server_session&&)      = default;
```

### stream

```c++
template <class... BufferArgs>
basic_server_session(multi_stream stream_, session_opts opts, BufferArgs&&... bargs);
```

Construct the server session by moving the supplied `multi_stream`.

## Member Functions

### async_detect_ssl

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

This function will timeout using `server_sesion.opts.timeout` as its duration.

### async_handshake

```c++
template <class HandshakeHandler>
auto
async_handshake(HandshakeHandler&& handler) ->
  typename boost::asio::async_result<std::decay_t<HandshakeHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type;
```

Performs the server portion of an SSL handshake.

This function will automatically [`upgrade`](./multi_stream.md#upgrade) the session's internal
stream object to SSL mode if the stream is not already. This function will `upgrade` with SSL
context found in the sessions `opts` member.

The handler function is invoked with an error code and the number of bytes consumed from the
underlying buffer during the handshake procedure. This occurs in the case of a user detecting an SSL
client upgrade request without physically performing it, thus filling the session's internal
buffers.

This function will timeout using `server_sesion.opts.timeout` as its duration.

### async_shutdown

```c++
template <class ShutdownHandler>
auto
async_shutdown(ShutdownHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ShutdownHandler>,
                                     void(boost::system::error_code)>::return_type;
```

This function will perform a TLS shutdown should it be required and then perform a TCP connection
close such that it avoids the TCP reset problem as highlighted by RFC 7230.

This function will timeout using `server_sesion.opts.timeout` as its duration.

### get_executor

```c++
auto
get_executor() -> executor_type;
```

Return a copy of the underlying executor. Serves as an executor hook.

### async_read_header

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

This function will timeout using `server_sesion.opts.timeout` as its duration.

### async_read

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

This function will timeout using `server_sesion.opts.timeout` as its duration.

### async_write_header

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

This function will timeout using `server_sesion.opts.timeout` as its duration.

### async_write

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

This function will timeout using `server_sesion.opts.timeout` as its duration.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
