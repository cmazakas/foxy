#ifndef FOXY_SERVER_SESSION_HPP_
#define FOXY_SERVER_SESSION_HPP_

#include "foxy/session.hpp"

namespace foxy
{
struct server_session : public session
{
public:
  server_session()                      = delete;
  server_session(server_session const&) = delete;
  server_session(server_session&&)      = default;

  explicit server_session(multi_stream stream_);
};

} // foxy

#endif // FOXY_SERVER_SESSION_HPP_
