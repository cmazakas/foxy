//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_SERVER_SESSION_HPP_
#define FOXY_SERVER_SESSION_HPP_

#include <foxy/session.hpp>

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
