//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/proxy.hpp>
#include <foxy/server_session.hpp>
#include <foxy/client_session.hpp>
#include <foxy/log.hpp>
#include <foxy/utility.hpp>

#include <foxy/detail/relay.hpp>
#include <foxy/detail/tunnel.hpp>

#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/error.hpp>

#include <boost/optional/optional.hpp>

#include <boost/asio/error.hpp>
#include <boost/asio/bind_executor.hpp>

#include <boost/assert.hpp>

#include <memory>
#include <iostream>

using boost::optional;
using boost::asio::ip::tcp;

namespace net   = boost::asio;
namespace http  = boost::beast::http;
namespace beast = boost::beast;

using namespace std::placeholders;

namespace
{
struct async_connect_op : boost::asio::coroutine
{
public:
  struct state
  {
    // our session with the current client
    //
    foxy::server_session session;

    // our session with the client's intended remote
    //
    foxy::client_session client;

    http::response_parser<http::empty_body> shutdown_parser;

    state(foxy::multi_stream stream, foxy::session_opts const& client_opts)
      : session(std::move(stream), {})
      , client(session.get_executor(), client_opts)
    {
    }
  };

  using executor_type = boost::asio::strand<typename ::foxy::session::executor_type>;

  std::unique_ptr<state> p_;
  executor_type          strand;

  async_connect_op(foxy::multi_stream stream, foxy::session_opts const& client_opts)
    : p_(std::make_unique<state>(std::move(stream), client_opts))
    , strand(p_->session.get_executor())
  {
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  };

  auto operator()(boost::system::error_code ec = {}, bool close_tunnel = false) -> void
  {
    BOOST_ASSERT(strand.running_in_this_thread());

    auto& s = *p_;
    BOOST_ASIO_CORO_REENTER(*this)
    {
      while (true) {
        BOOST_ASIO_CORO_YIELD
        ::foxy::detail::async_tunnel(s.session, s.client, std::move(*this));
        if (ec) { break; }
        if (close_tunnel) { break; }

        BOOST_ASIO_CORO_YIELD
        ::foxy::detail::async_relay(s.session, s.client, std::move(*this));
        if (ec) { break; }

        if (close_tunnel) { break; }
      }

      BOOST_ASIO_CORO_YIELD s.session.async_shutdown(std::move(*this));
      BOOST_ASIO_CORO_YIELD s.client.async_shutdown(std::move(*this));

      if (ec == boost::asio::error::eof) {
        // Rationale:
        // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
        ec.assign(0, ec.category());
      }

      if (ec) { foxy::log_error(ec, "proxy client shutdown"); }
    }
  }
};
} // namespace

foxy::proxy::proxy(boost::asio::io_context& io,
                   endpoint_type const&     endpoint,
                   bool                     reuse_addr,
                   foxy::session_opts       client_opts)
  : stream_(io.get_executor())
  , acceptor_(io.get_executor(), endpoint, reuse_addr)
  , strand_(stream_.get_executor())
  , client_opts_(std::move(client_opts))
{
}

auto
foxy::proxy::get_executor() -> executor_type
{
  return strand_;
}

auto
foxy::proxy::cancel() -> void
{
  strand_.post([self = shared_from_this()] { self->acceptor_.cancel(); }, std::allocator<char>());
}

auto
foxy::proxy::async_accept() -> void
{
  if (!acceptor_.is_open()) {
    std::cout << "foxy::proxy cannot accept on a closed ip::tcp::acceptor\n";
    return;
  }
  loop({});
}

auto
foxy::proxy::loop(boost::system::error_code ec) -> void
{
  BOOST_ASIO_CORO_REENTER(accept_coro_)
  {
    BOOST_ASIO_CORO_YIELD
    strand_.post([self = shared_from_this()] { self->loop({}); }, std::allocator<char>{});

    for (;;) {
      BOOST_ASIO_CORO_YIELD
      acceptor_.async_accept(
        stream_.plain(),
        boost::asio::bind_executor(strand_, std::bind(&proxy::loop, shared_from_this(), _1)));

      if (ec == boost::asio::error::operation_aborted) { break; }

      if (ec) {
        foxy::log_error(ec, "foxy::proxy::async_accept");
        continue;
      }

      boost::asio::post(async_connect_op(std::move(stream_), client_opts_));
    }
  }
}
