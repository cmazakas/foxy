# foxy::basic_multi_stream

## Include

```c++
#include <foxy/multi_stream.hpp>
```

## Synopsis

A class templated over a user-supplied
[`Stream`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/concepts/streams.html#beast.concepts.streams.AsyncStream)
that emulates dynamic polymorphism by reading/writing plain or encrypted bytes.

Can be used with the
[`boost::beast::test::stream`](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__test__stream.html)
but without SSL support.

## Declaration

```c++
template <class Stream>
struct basic_multi_stream;
```

## Exported Typedefs

```c++
using multi_stream = basic_multi_stream<boost::asio::ip::tcp::socket>;
```

## Member Typedefs

```c++
using stream_type     = Stream;
using ssl_stream_type = boost::beast::ssl_stream<stream_type>;
using executor_type   = typename Stream::executor_type;
```

## Constructors

### Default Constructors

```c++
basic_multi_stream()                          = delete;
basic_multi_stream(basic_multi_stream const&) = delete;
basic_multi_stream(basic_multi_stream&&)      = default;
```

### Plain

```c++
template <class Arg>
basic_multi_stream(Arg&& arg);
```

Construct the underlying `stream_type` by forwarding `arg` to its constructor.

### SSL

```c++
template <class Arg>
basic_multi_stream(Arg&& arg, boost::asio::ssl::context& ctx);
```

Construct the underlying `ssl_stream_type` by forwarding `arg` and `ctx` to its constructor.

## Member Functions

### plain

```c++
auto
plain() & noexcept -> stream_type&;
```

Return a reference to the `stream_type` member of the internal variant. If the session is using SSL,
this method returns the plain TCP portion of the layered SSL stream. This is typically only useful
for constructing the stream in SSL mode but using the plain TCP portions for connection setup
and teardown.

### ssl

```c++
auto
ssl() & noexcept -> ssl_stream_type&;
```

Returns a reference to the `ssl_stream_type` member of the internal variant. If the session is not
using SSL, this method invokes undefined behavior.

### is_ssl

```c++
auto
is_ssl() const noexcept -> bool;
```

Getter that returns returns whether or not the session was constructed with an SSL context and as
such is in SSL mode.

### get_executor

```c++
auto
get_executor() -> executor_type;
```

Returns a copy of the underlying stream's `executor_type`.

### upgrade

```c++
auto
upgrade(boost::asio::ssl::context& ctx) -> void;
```

Given a plain `multi_stream`, transform it to an `ssl_stream_type` using the supplied `ctx`.


### async_read_some

```c++
template <class MutableBufferSequence, class CompletionToken>
auto
async_read_some(MutableBufferSequence const& buffers, CompletionToken&& token);
```

Wrapper method that dispatches to the active stream's `async_read_some` method. Used to satisfy
[AsycReadStream](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/AsyncReadStream.html)
conditions.

### async_write_some

```c++
template <class ConstBufferSequence, class CompletionToken>
auto
async_write_some(ConstBufferSequence const& buffers, CompletionToken&& token);
```

Wrapper method that dispatches to the active stream's `async_write_some` method. Used to satisfy
[AsyncWriteStream](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/AsyncWriteStream.html)
conditions.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
