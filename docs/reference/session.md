# foxy::basic_session

```c++
#include <foxy/session.hpp>
```

## Synopsis

The most fundamental class in Foxy, the `basic_session` encapsulates a minimal subset of the classes
needed for using Beast.

All members are public so that users can make powerful and flexible abstractions without the library
becoming a hindrance.

The `basic_session` contains the [`foxy::basic_multi_stream`](./multi_stream.md#foxybasic_multi_stream)
so that it can act as a dual-stream type supporting both plain and encrypted data. It also contains
a user-specified buffer for use during parsing operations along with a timer that it uses to
determine when an operation should end.

The `basic_session` is configurable via its [`opts`](./session_opts.md#foxysession_opts) member.
Currently, only adjusting the timeouts has any direct effect. The client session, for example, will
only use the nested SSL context during its constructor and nowhere else. The server session will
only access the nested SSL context during its [`async_handshake`](./server_session.md#async_handshake)
function.

This class is used primarily for reading and writing HTTP/1.x messages.

The class is coded such that no timeout operations run in user-land code. Foxy ensures that before
each call to its async read and write member functions complete, the timer operations have been
cancelled and any pending operations have also been successfully run.

This guarantee enables the user to re-adjust the timeout between asynchronous operations without
ever having to worry about the state of the timer or any of its pending operations.

## Declaration

```c++
template <class Stream, class DynamicBuffer>
struct basic_session;
```

## Exported Typedefs

```c++
using session = basic_session<boost::asio::ip::tcp::socket, boost::beast::flat_buffer>;
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
basic_session()                     = delete;
basic_session(basic_session const&) = delete;
basic_session(basic_session&&)      = default;
```

### executor

```c++
template <class... BufferArgs>
basic_session(boost::asio::any_io_executor executor, session_opts opts_, BufferArgs&&... bargs);
```

Construct the session using the provided polymorphic executor. Forwards the executor to the
construction of the underlying `Stream`.

The construtor will instantiate the `DynamicBuffer` type with `bargs...`.

### io_context

```c++
template <class... BufferArgs>
basic_session(boost::asio::io_context& io, session_opts opts_ = {}, BufferArgs&&... bargs);
```

The "default" constructor for most use-cases.

If the session options contain an SSL context, the session will be constructed in SSL mode, i.e.
`session.stream.is_ssl()` returns `true`.

The construtor will instantiate the `DynamicBuffer` type with `bargs...`.

### stream_type

```c++
template <class... BufferArgs>
basic_session(stream_type stream_, session_opts opts_ = {}, BufferArgs&&... bargs);
```

Construct a session by moving the supplied `stream_` to the underlying session's `stream_type`.

The SSL mode of the session matches the SSL mode of the passed-in `stream_`.

The construtor will instantiate the `DynamicBuffer` type with `bargs...`.

## Member Functions

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

This function will timeout using `sesion.opts.timeout` as its duration.

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

This function will timeout using `sesion.opts.timeout` as its duration.

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

This function will timeout using `sesion.opts.timeout` as its duration.

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

This function will timeout using `sesion.opts.timeout` as its duration.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
