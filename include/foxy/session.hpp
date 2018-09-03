#ifndef FOXY_SESSION_HPP
#define FOXY_SESSION_HPP

#include "foxy/multi_stream.hpp"

#include <boost/asio/async_result.hpp>
#include <boost/asio/steady_timer.hpp>

#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace foxy
{

template <
  class Stream,
  class = std::enable_if_t<boost::beast::is_async_stream<Stream>::value>
>
struct basic_session
{
public:
  using stream_type     = Stream;
  using buffer_type     = boost::beast::flat_buffer;
  using timer_type      = boost::asio::steady_timer;

  stream_type stream;
  buffer_type buffer;
  timer_type  timer;

  basic_session()               = delete;
  basic_session(basic_session const&) = delete;
  basic_session(basic_session&&)      = default;

  explicit basic_session(boost::asio::io_context& io);
  explicit basic_session(stream_type stream_);

  using executor_type = decltype(stream.get_executor());

  auto get_executor() -> executor_type;

  template <class Parser, class ReadHandler>
  auto
  async_read_header(
    Parser&       parser,
    ReadHandler&& handler
  ) & -> BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t));

  template <class Parser, class ReadHandler>
  auto
  async_read(
    Parser&       parser,
    ReadHandler&& handler
  ) & -> BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t));

  template <class Serializer, class WriteHandler>
  auto
  async_write_header(
    Serializer&    serializer,
    WriteHandler&& handler
  ) & -> BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t));

  template <class Serializer, class WriteHandler>
  auto
  async_write(
    Serializer&    serializer,
    WriteHandler&& handler
  ) & -> BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t));
};

template <class Stream, class X>
foxy::basic_session<Stream, X>::basic_session(boost::asio::io_context& io)
: stream(io)
, timer(io)
{
}

template <class Stream, class X>
foxy::basic_session<Stream, X>::basic_session(stream_type stream_)
: stream(std::move(stream_))
, timer(stream.get_executor().context())
{
}

template <class Stream, class X>
auto foxy::basic_session<Stream, X>::get_executor() -> executor_type
{
  return stream.get_executor();
}

extern template struct basic_session<multi_stream>;

using session = basic_session<multi_stream>;

} // foxy

#include "foxy/impl/session.impl.hpp"

#endif // FOXY_SESSION_HPP
