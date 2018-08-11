#ifndef FOXY_SESSION_HPP
#define FOXY_SESSION_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/experimental/core/ssl_stream.hpp>

#include <boost/optional/optional.hpp>

namespace foxy
{
struct session
{
public:
  using stream_type     = boost::asio::ip::tcp::socket;
  using buffer_type     = boost::beast::flat_buffer;
  using timer_type      = boost::asio::steady_timer;
  using ssl_stream_type = boost::beast::ssl_stream<stream_type&>;

protected:
  stream_type stream_;
  buffer_type buffer_;
  timer_type  timer_;

  boost::optional<ssl_stream_type> ssl_stream_;

public:
  session()               = delete;
  session(session const&) = default;
  session(session&&)      = default;

  explicit session(boost::asio::io_context&);

  auto tcp() & noexcept -> stream_type&;
  auto ssl() & noexcept -> ssl_stream_type&;

  auto is_ssl() const noexcept -> bool;
};
} // foxy

#endif // FOXY_SESSION_HPP