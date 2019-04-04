//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_CLIENT_SESSION_HPP_
#define FOXY_CLIENT_SESSION_HPP_

#include <foxy/session.hpp>
#include <foxy/type_traits.hpp>
#include <foxy/shared_handler_ptr.hpp>
#include <foxy/detail/timed_op_wrapper.hpp>
#include <foxy/detail/timed_op_wrapper_v2.hpp>

#include <boost/system/error_code.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/coroutine.hpp>

#include <boost/asio/connect.hpp>

#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/core/bind_handler.hpp>

#include <boost/optional/optional.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <chrono>
#include <string>
#include <utility>
#include <memory>
#include <functional>
#include <iostream>

namespace foxy
{
struct client_session : public session
{
public:
  client_session()                      = delete;
  client_session(client_session const&) = default;
  client_session(client_session&&)      = default;

  explicit client_session(boost::asio::io_context& io, session_opts opts = {});

  // async_connect wraps asio::async_connect method and will invoke asio::async_connect with the
  // specified host and service parameters async_connect is SSL-aware and will automatically perform
  // the async handshake for the user, using the SSL context found in the session's opts
  //
  template <class ConnectHandler>
  auto
  async_connect(std::string host, std::string service, ConnectHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<ConnectHandler>,
                                       void(boost::system::error_code,
                                            boost::asio::ip::tcp::endpoint)>::return_type;

  // async_request will serialize the provided Request to the connected remote and then use the
  // provided ResponseParser to parse the response The user is expected to retrieve the underlying
  // message via the Beast http::parser interface
  //
  template <class Request, class ResponseParser, class RequestHandler>
  auto
  async_request(Request& request, ResponseParser& parser, RequestHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<RequestHandler>,
                                       void(boost::system::error_code)>::return_type;
};

} // namespace foxy

#include <foxy/impl/client_session/async_connect.impl.hpp>
#include <foxy/impl/client_session/async_request.impl.hpp>

#endif // FOXY_CLIENT_SESSION_HPP_
