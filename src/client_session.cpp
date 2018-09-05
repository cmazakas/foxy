#include "foxy/client_session.hpp"

foxy::client_session::client_session(boost::asio::io_context& io, boost::optional<session_opts> opts)
: session(io, std::move(opts))
{
}