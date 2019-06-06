//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_PROXY_HPP_
#define FOXY_PROXY_HPP_

#include <foxy/session.hpp>
#include <foxy/multi_stream.hpp>

#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/error.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/system/error_code.hpp>

#include <memory>
#include <array>

namespace foxy
{
// proxy is a simple TLS forward proxy
// It's intended to forward localhost traffic and then relay it for the client, performing any
// encryption along the way
//
struct proxy : public std::enable_shared_from_this<proxy>
{
public:
  using acceptor_type = boost::asio::ip::tcp::acceptor;
  using endpoint_type = boost::asio::ip::tcp::endpoint;
  using stream_type   = multi_stream;
  using executor_type = boost::asio::strand<typename stream_type::executor_type>;

private:
  stream_type          stream_;
  acceptor_type        acceptor_;
  executor_type        strand_;
  ::foxy::session_opts client_opts_;

  boost::asio::coroutine accept_coro_;

  auto loop(boost::system::error_code) -> void;

public:
  proxy()             = delete;
  proxy(proxy const&) = delete;
  proxy(proxy&&)      = default;

  proxy(boost::asio::io_context& io,
        endpoint_type const&     endpoint,
        bool                     reuse_addr  = false,
        session_opts             client_opts = {});

  auto
  get_executor() -> executor_type;

  auto
  async_accept() -> void;

  auto
  cancel() -> void;
};

} // namespace foxy

#endif // FOXY_PROXY_HPP_
