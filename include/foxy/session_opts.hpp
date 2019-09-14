//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_SESSION_OPTS_HPP_
#define FOXY_SESSION_OPTS_HPP_

#include <boost/asio/steady_timer.hpp>
#include <boost/asio/ssl/context.hpp>

#include <boost/optional/optional.hpp>

namespace foxy
{
struct session_opts
{
  using duration_type = typename boost::asio::steady_timer::duration;

  boost::optional<boost::asio::ssl::context&> ssl_ctx          = {};
  duration_type                               timeout          = std::chrono::seconds{1};
  bool                                        verify_peer_cert = true;
};
} // namespace foxy

#endif // FOXY_SESSION_OPTS_HPP_
