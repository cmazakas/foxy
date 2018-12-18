# Asio and Beast

Asio and Beast form the core machinery of Foxy. These libs were chosen very
selectively. This document aims to explore the reasons for choosing these
libraries as well as covering what relevant portions one must learn to
effectively use Foxy.

## Asio

Asio is a notoriously intimidating library and some past (and valid criticisms)
of it have been that it's all the pain of a low-level library with none of the
benefits. This is certainly true when compared to native platform-dependent
APIs but the reason why Foxy chose Asio is exactly because it's low-level from
an abstract perspective but is high-level in terms of being
platform-independent.

### Asio At a Glance

Asio is fundamentally just a collection of I/O objects, buffers, timers and
executors.

#### I/O Objects

The I/O objects that one would need to be successful with Foxy are the
[asio::ip::tcp::acceptor](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ip__tcp/acceptor.html),
[asio::ip::tcp::socket](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ip__tcp/socket.html)
and the [asio::ip::tcp::endpoint](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ip__tcp/endpoint.html).

An intuitive way to learn Asio's API for performing web development is to read
the [BSD-Asio Mapping](https://www.boost.org/doc/libs/release/doc/html/boost_asio/overview/networking/bsd_sockets.html).

#### Buffers

Buffers in Asio are where socket read operations store their data as well as
where socket write operations read their data from. Both Beast and Asio rely
heavily on this abstraction.

Asio creates a concept of a [DynamicBuffer](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/DynamicBuffer.html)
which defines the semantics for working with both input and output sequences.

Input and output sequences represent a sequence of either immutable or mutable
buffers ([ConstBufferSequence](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/ConstBufferSequence.html)
and [MutableBufferSequence](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/MutableBufferSequence.html)
respectively).

One should emphasize that Asio mentions the value type for the sequence is
something convertible to either [asio::const_buffer](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/const_buffer.html)
in the case of the `ConstBufferSequence` or convertible to [asio::mutable_buffer](https://www.boost.org/doc/libs/release/doc/html/boost_asio/reference/mutable_buffer.html)
for the `MutableBufferSequence`.

`const_buffer` and `mutable_buffer` are both view-like types in the modern C++
sense, i.e. they are both non-owning "views" over a contiguous region of bytes.
Intuitively, the `const_buffer`'s contents may not be modified while the
`mutable_buffer`'s contents can.

This means that when reading from a socket, one would need a
`MutableBufferSequence` and writing to a socket would require a
`ConstBufferSequence`.

For Foxy's purposes, the buffers provided by Beast are completely sufficient. In
fact, Foxy can survive entirely off of [beast::flat_buffer](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__flat_buffer.html)
which offers up a single element `MutableBufferSequence` / `ConstBufferSequence`.

For some, a single element range may be underwhelming so a true sequence of
buffers may be attained by using [beast::multi_buffer](https://www.boost.org/doc/libs/release/libs/beast/doc/html/beast/ref/boost__beast__multi_buffer.html).

#### Executors
