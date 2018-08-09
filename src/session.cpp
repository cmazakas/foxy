#include "foxy/session.hpp"

foxy::session::session(boost::asio::io_context& io)
: stream_(io)
, timer_(io)
{
}

auto foxy::session::tcp() & noexcept -> stream_type&
{
  return stream_;
}

auto foxy::session::ssl() & noexcept -> ssl_stream_type&
{
  return *ssl_stream_;
}

auto foxy::session::is_ssl() const noexcept -> bool
{
  return static_cast<bool>(ssl_stream_);
}