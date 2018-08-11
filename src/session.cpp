#include "foxy/session.hpp"

foxy::session::session(boost::asio::io_context& io)
: stream(io)
, timer(io)
{
}