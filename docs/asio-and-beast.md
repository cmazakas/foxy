# Asio and Beast

Asio and Beast form the core machinery of Foxy. This document aims to explore the reasons for
choosing these libraries as well as covering what relevant portions one must learn to effectively
use Foxy.

## Asio

Asio is a notoriously intimidating library and some past (and valid criticisms) of it have been that
it's all the pain of a low-level library with none of the benefits. This is certainly true when
compared to native platform-dependent APIs but the reason why Foxy chose Asio is exactly because
it's low-level from an abstract perspective but is high-level in terms of being
platform-independent.

Asio gives programmers a set of types that they can use to compose higher-order abstractions with
Beast being one of the most notable.

### Asio At a Glance

Asio is fundamentally just a collection of networking I/O objects, buffers, timers and executors.

#### Networking I/O Objects

The I/O objects that one would need to be successful with Foxy are the
[asio::ip::tcp::acceptor](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ip__tcp/acceptor.html),
[asio::ip::tcp::socket](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ip__tcp/socket.html)
and the [asio::ip::tcp::endpoint](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ip__tcp/endpoint.html).

An intuitive way to learn Asio's API for performing web development is to read the
[BSD-Asio Mapping](https://www.boost.org/doc/libs/release/doc/html/boost_asio/overview/networking/bsd_sockets.html).

Understanding how to use these types beyond simply constructing them has mostly been abstracted away
by both Beast and Foxy.

#### Buffers

Buffers in Asio are where socket read operations store their data as well as where socket write
operations read their data from. Both Beast and Asio rely heavily on this abstraction.

Asio creates a concept of a [DynamicBuffer](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/DynamicBuffer.html)
which defines the semantics for working with both input and output sequences.

Input and output sequences represent a sequence of either immutable or mutable buffers
([ConstBufferSequence](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ConstBufferSequence.html)
and [MutableBufferSequence](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/MutableBufferSequence.html)
respectively).

One should emphasize that Asio mentions the value type for the sequence is something convertible to
either [asio::const_buffer](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/const_buffer.html)
in the case of the `ConstBufferSequence` or convertible to [asio::mutable_buffer](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/mutable_buffer.html)
for the `MutableBufferSequence`.

`const_buffer` and `mutable_buffer` are both view-like types in the modern C++ sense, i.e. they are
both non-owning "views" over a contiguous region of bytes. Intuitively, the `const_buffer`'s
contents may not be modified while the `mutable_buffer`'s contents can.

This means that when reading from a socket, one would need a `MutableBufferSequence` and writing to
a socket would require a `ConstBufferSequence`.

For Foxy's purposes, the buffers provided by Beast are completely sufficient. In fact, Foxy can
survive entirely off of [beast::flat_buffer](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__flat_buffer.html)
which offers up a single element `MutableBufferSequence` / `ConstBufferSequence`.

For some, a single element range may be underwhelming so a true sequence of buffers may be attained
by using [beast::multi_buffer](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__multi_buffer.html).

But why is the `DynamicBuffer` concept needed in Asio at all?

In practice, Asio code looks a bit like this:
```cpp
socket.async_read_some(boost::asio::buffer(data, size), handler);
```

Here, our socket reads into the `MutableBufferSequence` formed by the `boost::asio::buffer(data, size)`
call which returns a `mutable_buffer`.

Asio fully abstracts away any of the platform-specific code but will only go so far as forming the
pipe itself. We still need to store the data that comes through the socket and this is a feature,
not a bug.

Asio gives us full control over the layout of the memory regions we need to successfully work with
the network. For example, we _can_ simply choose to read into a flat contiguous buffer (via Beast's
`flat_buffer`) or we can use a type of paging system where we have multiple storage regions in a
type of linked list (like Beast's `multi_buffer`).

This is part of Asio's interface that gives it so much power and the `DynamicBuffer` concept is a
useful formalization of how one uses both `Mutable` and `ConstBufferSequence`s.

#### Executors


[To ToC](./intro.md#Table\ of\ Contents)
