## `foxy::client_session`

## Include

```c++
#include <foxy/client_session.hpp>
```

## Synopsis

The `client_session` is one of Foxy's main workhorses. It gives users an easy to connect to a
remote server and then send and receive messages.

## Declaration

```c++
struct client_session : public session;
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
client_session()                      = delete;
client_session(client_session const&) = delete;
client_session(client_session&&)      = default;
```

#### `io_context`

```c++
explicit client_session(boost::asio::io_context& io, session_opts opts = {});
```

## Member Functions

#### async_connect

```c++
template <class ConnectHandler>
auto
async_connect(std::string host, std::string service, ConnectHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ConnectHandler>,
                                      void(boost::system::error_code,
                                          boost::asio::ip::tcp::endpoint)>::return_type;
```

Asynchronously connect to the remote denoted by the host and service. This function performs forward
DNS resolution of the host and then forms a TCP connection over one of the associated endpoints.

Once the connection has been established, this function will then perform an SSL handshake if the
session options contain an SSL context.

This function will timeout.

#### async_request

```c++
template <class Request, class ResponseParser, class RequestHandler>
auto
async_request(Request& request, ResponseParser& parser, RequestHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<RequestHandler>,
                                      void(boost::system::error_code)>::return_type;
```

Write the provided request object to the underlying stream and then read the response back using
the provided parser.

* `Request` = `boost::beast::http::request` | `boost::beast::http::requset_serializer`
* `ResponseParser` = `boost::beast::http::response` | `boost::beast::http::response_parser`

This function will timeout.

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

This function will timeout.

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
