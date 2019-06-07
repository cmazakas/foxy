## `foxy::basic_session`

```c++
#include <foxy/session.hpp>
```

## Synopsis

The most fundamental class in Foxy, the `basic_session` encapsulates a minimal subset of the classes
needed for using Beast and Asio.

All members are public so that users can make powerful and flexible abstractions without the library
becoming a hindrance.

The `basic_session` contains the `foxy::basic_multi_stream` so that it can act as a dual-stream type
supporting both plain and encrypted data. It also contains a buffer for use during parsing and
serialization operations as well as a timer that it uses to determine when an operation should end.

The `basic_session` is configurable via its `foxy::session_opts`. Currently, only adjusting the
timeouts has any direct effect. Mutating the nested SSL context may result in an inconsistent state
of the session.

This class is used primarily for reading and writing HTTP messages.

The class is coded such that no timeout operations run in user-land code. Foxy ensures that before
each call to its async read and write member functions completes, the timer operation has been
cancelled and that it has also been successfully run.

This guarantee enables the user to re-adjust the timeout between asynchronous operations without
ever having to worry about the state of the timer or any of its pending operations.

## Declaration

```c++
template <class Stream>
struct basic_session;
```

## Exported Typedefs

```c++
using session = basic_session<
  boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::io_context::executor_type>>;
```

## Member Typedefs

```c++
using stream_type   = ::foxy::basic_multi_stream<Stream>;
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
basic_session()                     = delete;
basic_session(basic_session const&) = delete;
basic_session(basic_session&&)      = default;
```

#### `io_context`

```c++
explicit basic_session(boost::asio::io_context& io, session_opts opts_ = {});
```

The "default" constructor for most use-cases.

If the session options contain an SSL context, the session will be constructed in SSL mode.

#### `stream_type`

```c++
explicit basic_session(stream_type stream_, session_opts opts_ = {});
```

Construct a session by moving the supplied `stream_` to the underlying session's `stream_type`.

The SSL mode of the session matches the SSL mode of the passed-in `stream_`.

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
