#ifndef FOXY_SESSION_HPP
#define FOXY_SESSION_HPP

#include "foxy/multi_stream.hpp"

#include <boost/beast/core/flat_buffer.hpp>
#include <boost/asio/steady_timer.hpp>

namespace foxy
{
struct session
{
public:
  using stream_type     = ::foxy::multi_stream;
  using buffer_type     = boost::beast::flat_buffer;
  using timer_type      = boost::asio::steady_timer;
  using ssl_stream_type = boost::beast::ssl_stream<stream_type&>;

  stream_type stream;
  buffer_type buffer;
  timer_type  timer;

  session()               = delete;
  session(session const&) = default;
  session(session&&)      = default;

  explicit session(boost::asio::io_context&);
};
} // foxy

#endif // FOXY_SESSION_HPP