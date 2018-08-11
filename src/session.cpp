#include "foxy/session.hpp"

foxy::session::session(boost::asio::io_context& io)
: stream(io)
, timer(io)
{
}

auto foxy::session::get_executor() -> executor_type
{
  return stream.get_executor();
}