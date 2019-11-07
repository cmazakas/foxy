//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_TUNNEL_HPP_
#define FOXY_DETAIL_TUNNEL_HPP_

#include <foxy/client_session.hpp>
#include <foxy/server_session.hpp>
#include <foxy/type_traits.hpp>
#include <foxy/parse_uri.hpp>
#include <foxy/detail/relay.hpp>

#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/status.hpp>

#include <boost/beast/core/detect_ssl.hpp>

#include <boost/optional/optional.hpp>

namespace foxy
{
namespace detail
{
template <class TunnelHandler>
struct tunnel_op
  : boost::beast::stable_async_base<TunnelHandler, typename ::foxy::session::executor_type>,
    boost::asio::coroutine
{
  struct state
  {
    boost::optional<boost::beast::http::request_parser<boost::beast::http::empty_body>> parser;

    boost::optional<
      boost::beast::http::response<boost::beast::http::string_body, boost::beast::http::fields>>
      response;

    foxy::basic_uri_parts<char> uri_parts;

    boost::tribool is_ssl;

    bool is_authority = false;
    bool is_connect   = false;
    bool is_absolute  = false;
    bool is_http      = false;

    bool close_tunnel = false;
  };

  ::foxy::server_session& server;
  ::foxy::client_session& client;
  state&                  s;

public:
  tunnel_op()                 = delete;
  tunnel_op(tunnel_op const&) = default;
  tunnel_op(tunnel_op&&)      = default;

  tunnel_op(foxy::server_session& server_, TunnelHandler handler, foxy::client_session& client_)
    : boost::beast::stable_async_base<TunnelHandler, typename ::foxy::session::executor_type>(
        std::move(handler),
        server_.get_executor())
    , server(server_)
    , client(client_)
    , s(boost::beast::allocate_stable<state>(*this))
  {
    (*this)({}, 0, false);
  }

  struct on_connect_t
  {
  };

  struct on_relay_t
  {
  };

  struct on_detect_t
  {
  };

  auto
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred,
             bool const                is_continuation = true) -> void;

  auto
  operator()(on_connect_t, boost::system::error_code ec) -> void;

  auto
  operator()(on_relay_t, boost::system::error_code ec, bool close_tunnel) -> void;

  auto
  operator()(on_detect_t, boost::system::error_code ec, boost::tribool is_ssl_) -> void;
};

template <class TunnelHandler>
auto
tunnel_op<TunnelHandler>::operator()(on_connect_t, boost::system::error_code ec) -> void
{
  (*this)(ec, 0);
}

template <class TunnelHandler>
auto
tunnel_op<TunnelHandler>::operator()(on_relay_t, boost::system::error_code ec, bool) -> void
{
  (*this)(ec, 0);
}

template <class TunnelHandler>
auto
tunnel_op<TunnelHandler>::
operator()(on_detect_t, boost::system::error_code ec, boost::tribool is_ssl_) -> void
{
  s.is_ssl = is_ssl_;
  (*this)(ec, 0);
}

template <class TunnelHandler>
auto
tunnel_op<TunnelHandler>::operator()(boost::system::error_code ec,
                                     std::size_t               bytes_transferred,
                                     bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;
  using boost::beast::bind_front_handler;

  namespace net  = boost::asio;
  namespace http = boost::beast::http;

  BOOST_ASIO_CORO_REENTER(*this)
  {
    while (true) {
      s.parser.emplace();

      s.response.emplace();

      BOOST_ASIO_CORO_YIELD
      server.async_read_header(*s.parser, std::move(*this));

      if (ec) { goto upcall; }

      s.uri_parts = foxy::parse_uri(s.parser->get().target());

      s.is_authority = s.uri_parts.is_authority();
      s.is_connect   = s.parser->get().method() == http::verb::connect;
      s.is_absolute  = s.uri_parts.is_absolute();
      s.is_http      = s.uri_parts.is_http();

      if (s.is_connect && s.is_authority && !s.parser->keep_alive()) {
        s.close_tunnel = true;

        s.response->result(http::status::bad_request);
        s.response->body() = "CONNECT semantics require a persistent connection\n\n";
        s.response->prepare_payload();

        BOOST_ASIO_CORO_YIELD
        server.async_write(*s.response, std::move(*this));

        if (ec) { goto upcall; }
        break;
      }

      if ((s.is_connect && s.is_authority) || (s.is_absolute && s.is_http)) {
        BOOST_ASIO_CORO_YIELD
        {
          auto const scheme =
            client.stream.is_ssl() ? boost::string_view("https") : boost::string_view("http");

          auto port = s.uri_parts.port().size() == 0 ? static_cast<std::string>(scheme)
                                                     : static_cast<std::string>(s.uri_parts.port());

          client.async_connect(static_cast<std::string>(s.uri_parts.host()), std::move(port),
                               bind_front_handler(std::move(*this), on_connect_t{}));
        }

        if (ec) {
          s.response->result(http::status::bad_request);
          s.response->body() =
            "Unable to connect to the remote at: " + static_cast<std::string>(s.uri_parts.host()) +
            "\nError code: " + ec.message() + "\n\n";

          s.response->prepare_payload();

          BOOST_ASIO_CORO_YIELD
          server.async_write(*s.response, std::move(*this));

          if (ec) { goto upcall; }
          if (s.parser->get().keep_alive()) { continue; }
          break;
        }

      } else {
        s.response->result(http::status::bad_request);
        s.response->body() =
          "Malformed client request. Use either CONNECT <authority-uri> or <verb> <absolute-uri>";
        s.response->prepare_payload();

        BOOST_ASIO_CORO_YIELD
        server.async_write(*s.response, std::move(*this));

        if (ec) { goto upcall; }
        if (s.parser.get().keep_alive()) { continue; }
        break;
      }

      if (s.is_absolute && s.is_http) {
        s.parser->get().keep_alive(false);

        BOOST_ASIO_CORO_YIELD
        {
          auto const path =
            s.uri_parts.path().size() == 0
              ? (s.parser->get().method() == http::verb::options ? boost::string_view("*")
                                                                 : boost::string_view("/"))
              : s.uri_parts.path();

          auto target = static_cast<std::string>(path);
          if (s.uri_parts.query().size() > 0) {
            target += "?";
            target += static_cast<std::string>(s.uri_parts.query());
          }

          auto hostname = static_cast<std::string>(s.uri_parts.host());
          if (s.uri_parts.port().size() > 0) {
            hostname += ":";
            hostname += static_cast<std::string>(s.uri_parts.port());
          }

          s.parser->get().target(target);
          s.parser->get().set(http::field::host, hostname);

          async_relay(server, client, std::move(*s.parser),
                      bind_front_handler(std::move(*this), on_relay_t{}));
        }

        if (ec) { goto upcall; }
        s.close_tunnel = true;
        break;
      }

      s.response->result(http::status::ok);

      BOOST_ASIO_CORO_YIELD
      server.async_write(*s.response, std::move(*this));

      if (ec) {
        s.close_tunnel = true;
        goto upcall;
      }

      s.close_tunnel = false;
      break;
    }

    if (!ec && !s.close_tunnel) {
      BOOST_ASIO_CORO_YIELD
      boost::beast::async_detect_ssl(server.stream.plain(), server.buffer,
                                     bind_front_handler(std::move(*this), on_detect_t{}));
    }

    {
      auto const close_tunnel = s.close_tunnel;
      return this->complete(is_continuation, boost::system::error_code(), close_tunnel);
    }

  upcall:
    auto const close_tunnel = s.close_tunnel;
    this->complete(is_continuation, ec, close_tunnel);
  }
}

struct run_async_tunnel_op
{
  template <class Handler>
  auto
  operator()(Handler&& handler, foxy::server_session& server, foxy::client_session& client) -> void
  {
    tunnel_op<Handler>(server, std::forward<Handler>(handler), client);
  }
};

template <class CompletionToken>
auto
async_tunnel(foxy::server_session& server, foxy::client_session& client, CompletionToken&& token) ->
  typename boost::asio::async_result<std::decay_t<CompletionToken>,
                                     void(boost::system::error_code, bool)>::return_type
{
  return boost::asio::async_initiate<CompletionToken, void(boost::system::error_code, bool)>(
    run_async_tunnel_op{}, token, server, client);
}

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_TUNNEL_HPP_
