#include "foxy/session.hpp"

foxy::session::session(boost::asio::io_context& io)
: stream(io)
, timer(io)
{
}

foxy::session::session(stream_type stream_)
: stream(std::move(stream_))
, timer(stream.get_executor().context())
{
}

auto foxy::session::get_executor() -> executor_type
{
  return stream.get_executor();
}