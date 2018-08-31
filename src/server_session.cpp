#include "foxy/server_session.hpp"

foxy::server_session::server_session(multi_stream stream_)
: session(std::move(stream_))
{
}
