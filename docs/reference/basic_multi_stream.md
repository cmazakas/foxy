# `foxy::basic_multi_stream`

## Declaration
```c++
namespace foxy
{
template <class Stream>
struct basic_multi_stream;
}
```

## Typedefs

```c++
using stream_type     = Stream;
using ssl_stream_type = boost::beast::ssl_stream<stream_type>;
using executor_type   = boost::asio::io_context::executor_type;
```

## Constructors

```c++
template <class Arg>
basic_multi_stream(Arg&& arg);
```

Construct the underlying `stream_type` by forwarding `arg` to its constructor.

```c++
template <class Arg>
basic_multi_stream(Arg&& arg, boost::asio::ssl::context& ctx);
```

Construct the underlying `ssl_stream_type` by forwarding `arg` and `ctx` to its constructor.

## Member Functions
```c++
auto
plain() & noexcept -> stream_type&;
```

Return a reference to the `stream_type` member of the internal variant. If the session is using SSL,
this method invokes undefined behavior.

```c++
auto
ssl() & noexcept -> ssl_stream_type&;
```

Returns a reference to the `ssl_stream_type` member of the internal variant. If the session is not
using SSL, this method invokes undefined behavior.

```c++
auto
is_ssl() const noexcept -> bool;
```

Getter that returns returns whether or not the session was constructed with an SSL context and as
such is in SSL mode.

```c++
auto
get_executor() -> executor_type;
```

Returns a copy of the underlying stream's `executor_type`.

```c++
template <class MutableBufferSequence, class CompletionToken>
auto
async_read_some(MutableBufferSequence const& buffers, CompletionToken&& token);
```

Wrapper method that dispatches to the active stream's `async_read_some` method. Used to satisfy
[AsycReadStream](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/AsyncReadStream.html)
conditions.

```c++
template <class ConstBufferSequence, class CompletionToken>
auto
async_write_some(ConstBufferSequence const& buffers, CompletionToken&& token);
```

Wrapper method that dispatches to the active stream's `async_write_some` method. Used to satisfy
[AsyncWriteStream](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/AsyncWriteStream.html)
conditions.

To [ToC](./index.md#Table-of-Contents)
