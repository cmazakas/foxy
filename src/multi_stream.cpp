#include "foxy/multi_stream.hpp"

foxy::multi_stream::multi_stream(boost::asio::io_context& io)
: stream_(io)
{
}

auto foxy::multi_stream::plain() & noexcept -> stream_type&
{
  return stream_;
}

auto foxy::multi_stream::ssl() & noexcept -> ssl_stream_type&
{
  return *ssl_stream_;
}

auto foxy::multi_stream::is_ssl() const noexcept -> bool
{
  return static_cast<bool>(ssl_stream_);
}

auto foxy::multi_stream::get_executor() -> boost::asio::io_context::executor_type
{
  return stream_.get_executor();
}