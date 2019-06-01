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
      : session(std::move(stream))
      , client(session.get_executor().context(), client_opts)
    {
    }
  };

  std::unique_ptr<state>                                      p_;
  boost::asio::strand<boost::asio::io_context::executor_type> strand;

  async_connect_op(foxy::multi_stream stream, foxy::session_opts const& client_opts)
    : p_(std::make_unique<state>(std::move(stream), client_opts))
    , strand(p_->session.get_executor().context().get_executor())
  {
  }

  using executor_type =
    boost::asio::strand<decltype(std::declval<::foxy::session&>().get_executor())>;

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  };

  auto
  operator()(boost::system::error_code ec, bool close_tunnel) -> void
  {
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

      // http rfc 7230 section 6.6 Tear-down
      // -----------------------------------
      // To avoid the TCP reset problem, servers typically close a connection
      // in stages.  First, the server performs a half-close by closing only
      // the write side of the read/write connection.  The server then
      // continues to read from the connection until it receives a
      // corresponding close by the client, or until the server is reasonably
      // certain that its own TCP stack has received the client's
      // acknowledgement of the packet(s) containing the server's last
      // response.  Finally, the server fully closes the connection.
      //
      s.session.stream.plain().shutdown(tcp::socket::shutdown_send, ec);

      BOOST_ASIO_CORO_YIELD
      s.session.async_read(s.shutdown_parser, std::move(*this));

      if (ec && ec != http::error::end_of_stream) {
        foxy::log_error(ec, "foxy::proxy::tunnel::shutdown_wait_for_eof_error");
      }

      s.session.stream.plain().shutdown(tcp::socket::shutdown_receive, ec);
      s.session.stream.plain().close(ec);

      if (s.client.stream.is_ssl()) {
        BOOST_ASIO_CORO_YIELD
        s.client.stream.ssl().async_shutdown(std::bind(std::move(*this), _1, true));

        if (ec == boost::asio::error::eof) {
          // Rationale:
          // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
          ec.assign(0, ec.category());
        }

        if (ec) { foxy::log_error(ec, "ssl client shutdown"); }

      } else {
        s.client.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
        s.client.stream.plain().close(ec);
      }
    }
  }
};
} // namespace

foxy::proxy::proxy(boost::asio::io_context& io,
                   endpoint_type const&     endpoint,
                   bool                     reuse_addr,
                   foxy::session_opts       client_opts)
  : stream_(io)
  , acceptor_(io, endpoint, reuse_addr)
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

      async_connect_op(std::move(stream_), client_opts_)({}, false);
    }
  }
}
