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

  std::unique_ptr<state> p_;

  async_connect_op(foxy::multi_stream stream, foxy::session_opts const& client_opts);

  auto
  operator()(boost::system::error_code ec, bool close) -> void;
};
} // namespace

foxy::proxy::proxy(boost::asio::io_context& io,
                   endpoint_type const&     endpoint,
                   bool                     reuse_addr,
                   foxy::session_opts       client_opts)
  : stream_(io)
  , acceptor_(io, endpoint, reuse_addr)
  , client_opts_(std::move(client_opts))
{
}

auto
foxy::proxy::get_executor() -> executor_type
{
  return stream_.get_executor();
}

auto
foxy::proxy::cancel(boost::system::error_code& ec) -> void
{
  acceptor_.cancel(ec);
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
      acceptor_.async_accept(stream_.plain(), std::bind(&proxy::loop, shared_from_this(), _1));

      if (ec == boost::asio::error::operation_aborted) { break; }

      if (ec) {
        foxy::log_error(ec, "foxy::proxy::async_accept");
        continue;
      }

      async_connect_op(std::move(stream_), client_opts_)({}, false);
    }
  }
}

namespace
{
async_connect_op::async_connect_op(foxy::multi_stream stream, foxy::session_opts const& client_opts)
  : p_(std::make_unique<state>(std::move(stream), client_opts))
{
}

auto
async_connect_op::operator()(boost::system::error_code ec, bool close_tunnel) -> void
{
  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    while (true) {
      BOOST_ASIO_CORO_YIELD
      ::foxy::detail::async_tunnel(s.session, s.client, std::move(*this));
      if (ec) {
        // do shutdown stuff
        break;
      }

      if (close_tunnel) { break; }

      BOOST_ASIO_CORO_YIELD
      ::foxy::detail::async_relay(s.session, s.client, std::move(*this));
      if (ec) {
        // do shutdown stuff
        break;
      }

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

    s.client.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
    s.client.stream.plain().close(ec);

    //   while (true) {
    //     if (s.parser) { s.parser = boost::none; }
    //     s.parser.emplace();

    //     BOOST_ASIO_CORO_YIELD
    //     s.session.async_read(*s.parser, std::move(*this));

    //     if (ec == http::error::end_of_stream) { break; }

    //     if (ec == http::error::unexpected_body) {
    //       BOOST_ASIO_CORO_YIELD
    //       write_error(http::status::bad_request,
    //                   "Messages with bodies are not supported for establishing a "
    //                   "tunnel\n\n");

    //       s.err_response = {};
    //       if (ec) {
    //         foxy::log_error(ec, "foxy::proxy::async_accept_op::unexpected_body::write_error");
    //         BOOST_ASIO_CORO_YIELD break;
    //       }

    //       s.session.buffer.consume(s.session.buffer.size());
    //       continue;
    //     }

    //     if (ec) {
    //       foxy::log_error(ec, "foxy::proxy::async_connect_op::async_read::read_error");
    //       BOOST_ASIO_CORO_YIELD break;
    //     }

    //     // we can only form a proper tunnel over a persistent connection
    //     //
    //     if (!s.parser->get().keep_alive()) {
    //       BOOST_ASIO_CORO_YIELD
    //       write_error(http::status::bad_request,
    //                   "Connection must be persistent to allow proper tunneling\n\n");

    //       s.err_response = {};
    //       if (ec) {
    //         foxy::log_error(ec,
    //         "foxy::proxy::async_accept_op::non_keepalive_request::write_error");
    //         BOOST_ASIO_CORO_YIELD break;
    //       }

    //       break;
    //     }

    //     // a CONNECT must be used to signify tunnel semantics
    //     //
    //     if (s.parser->get().method() != http::verb::connect) {
    //       BOOST_ASIO_CORO_YIELD
    //       write_error(http::status::method_not_allowed,
    //                   "Invalid request method. Only CONNECT is supported\n\n");

    //       s.err_response = {};
    //       if (ec) {
    //         foxy::log_error(ec, "foxy::proxy::async_accept_op::non_connect_verb::write_error");
    //         BOOST_ASIO_CORO_YIELD break;
    //       }

    //       continue;
    //     }

    //     // extract request target and attempt to form the tunnel
    //     //
    //     BOOST_ASIO_CORO_YIELD
    //     {
    //       auto request = s.parser->release();
    //       auto target  = request.target();

    //       auto host_and_port = foxy::parse_authority_form(target);

    //       s.client.async_connect(std::move(std::get<0>(host_and_port)),
    //                              std::move(std::get<1>(host_and_port)),
    //                              beast::bind_handler(std::move(*this), on_connect_t{}, _1, _2));
    //     }

    //     // TODO: support 504 for `operation_aborted` error code
    //     //
    //     if (ec) {
    //       BOOST_ASIO_CORO_YIELD
    //       write_error(http::status::bad_request,
    //                   std::string("Unable to establish connection with the remote\n") +
    //                     "Error: " + ec.message() + "\n\n");

    //       s.err_response = {};
    //       if (ec) {
    //         foxy::log_error(ec, "foxy::proxy::async_accept_op::failed_connect::write_error");
    //         BOOST_ASIO_CORO_YIELD break;
    //       }

    //       continue;
    //     }

    //     BOOST_ASIO_CORO_YIELD
    //     s.session.async_write(s.tunnel_res, std::move(*this));
    //     if (ec) {
    //       foxy::log_error(ec,
    //                       "foxy::proxy::async_connect_op::failed_write_back_"
    //                       "successful_tunnel_res");
    //       BOOST_ASIO_CORO_YIELD break;
    //     }

    //     // at this point, we can safely use our `client_session` object for
    //     // tunneling requests from the client to the upstream
    //     //
    //     should_tunnel = true;
    //     BOOST_ASIO_CORO_YIELD break;
    //   }

    //   // http rfc 7230 section 6.6 Tear-down
    //   // -----------------------------------
    //   // To avoid the TCP reset problem, servers typically close a connection
    //   // in stages.  First, the server performs a half-close by closing only
    //   // the write side of the read/write connection.  The server then
    //   // continues to read from the connection until it receives a
    //   // corresponding close by the client, or until the server is reasonably
    //   // certain that its own TCP stack has received the client's
    //   // acknowledgement of the packet(s) containing the server's last
    //   // response.  Finally, the server fully closes the connection.
    //   //
    //   s.session.stream.plain().shutdown(tcp::socket::shutdown_send, ec);

    //   if (s.parser) { s.parser = boost::none; }
    //   s.parser.emplace();

    //   BOOST_ASIO_CORO_YIELD
    //   s.session.async_read(*s.parser, std::move(*this));

    //   if (ec && ec != http::error::end_of_stream) {
    //     foxy::log_error(ec, "foxy::proxy::tunnel::shutdown_wait_for_eof_error");
    //   }

    //   s.session.stream.plain().shutdown(tcp::socket::shutdown_receive, ec);
    //   s.session.stream.plain().close(ec);
  }

  // if (!s.connect_coro.is_complete() || ec || !should_tunnel) { return; }
  // (*this)(on_tunnel_t{}, {}, false);
}

// void
// async_connect_op::operator()(on_tunnel_t, boost::system::error_code ec, bool const should_close)
// {
// auto& s = *p_;
// BOOST_ASIO_CORO_REENTER(s.tunnel_coro)
// {
//   while (!should_close && !ec) {
//     BOOST_ASIO_CORO_YIELD
//     ::foxy::detail::async_relay(s.session, s.client,
//                                 beast::bind_handler(std::move(*this), on_tunnel_t{}, _1,
//                                 _2));
//   }

//   // if we close one connection, we close the other as well
//   //
//   s.session.stream.plain().shutdown(tcp::socket::shutdown_send, ec);

//   if (s.parser) { s.parser = boost::none; }
//   s.parser.emplace();

//   BOOST_ASIO_CORO_YIELD
//   s.session.async_read(*s.parser,
//                        beast::bind_handler(std::move(*this), on_close_read_t{}, _1, _2));

//   if (ec && ec != http::error::end_of_stream) {
//     foxy::log_error(ec, "foxy::proxy::tunnel::shutdown_wait_for_eof_error");
//   }

//   s.session.stream.plain().shutdown(tcp::socket::shutdown_receive, ec);
//   s.session.stream.plain().close(ec);

//   if (s.client.stream.is_ssl()) {
//     BOOST_ASIO_CORO_YIELD
//     s.client.stream.ssl().async_shutdown(
//       beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, true));

//     if (ec == boost::asio::error::eof) {
//       // Rationale:
//       //
//       http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
//       ec.assign(0, ec.category());
//     }

//     if (ec) { foxy::log_error(ec, "ssl client shutdown"); }

//     BOOST_ASIO_CORO_YIELD break;
//   }

//   s.client.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
//   s.client.stream.plain().close(ec);
// }
// }

} // namespace
