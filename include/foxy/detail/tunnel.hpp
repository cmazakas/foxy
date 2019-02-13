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
#include <foxy/uri_parts.hpp>
#include <foxy/detail/relay.hpp>

#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/status.hpp>

#include <boost/optional/optional.hpp>

#include <iostream>

namespace foxy
{
namespace detail
{
template <class TunnelHandler>
struct tunnel_op : boost::asio::coroutine
{
public:
  using executor_type = boost::asio::associated_executor_t<
    TunnelHandler,
    decltype((std::declval<foxy::basic_session<boost::asio::ip::tcp::socket>&>().get_executor()))>;

  using allocator_type = boost::asio::associated_allocator_t<TunnelHandler>;

private:
  struct state
  {
    foxy::server_session& server;
    foxy::client_session& client;

    boost::optional<
      boost::beast::http::request_parser<boost::beast::http::empty_body, allocator_type>>
      parser;

    boost::optional<boost::beast::http::response<boost::beast::http::string_body,
                                                 boost::beast::http::basic_fields<allocator_type>>>
      response;

    foxy::uri_parts uri_parts;

    bool is_authority = false;
    bool is_connect   = false;
    bool is_absolute  = false;
    bool is_http      = false;

    bool close_tunnel = false;

    boost::asio::executor_work_guard<decltype(server.get_executor())> work;

    explicit state(TunnelHandler const&  handler,
                   foxy::server_session& server_,
                   foxy::client_session& client_)
      : server(server_)
      , client(client_)
      , parser(boost::in_place_init,
               std::piecewise_construct,
               std::make_tuple(),
               std::make_tuple(boost::asio::get_associated_allocator(handler)))
      , response(boost::in_place_init,
                 std::piecewise_construct,
                 std::make_tuple(boost::asio::get_associated_allocator(handler)),
                 std::make_tuple(boost::asio::get_associated_allocator(handler)))
      , work(server.get_executor())
    {
    }
  };

  boost::beast::handler_ptr<state, TunnelHandler> p_;

public:
  tunnel_op()                 = delete;
  tunnel_op(tunnel_op const&) = default;
  tunnel_op(tunnel_op&&)      = default;

  template <class DeducedHandler>
  tunnel_op(foxy::server_session& server, foxy::client_session& client, DeducedHandler&& handler)
    : p_(std::forward<DeducedHandler>(handler), server, client)
  {
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->server.get_executor());
  }

  auto
  get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  struct on_connect_t
  {
  };

  struct on_relay_t
  {
  };

  auto
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred,
             bool const                is_continuation = true) -> void;

  auto
  operator()(on_connect_t, boost::system::error_code ec, boost::asio::ip::tcp::endpoint) -> void;

  auto
  operator()(on_relay_t, boost::system::error_code ec, bool close_tunnel) -> void;
};

template <class TunnelHandler>
auto
tunnel_op<TunnelHandler>::
operator()(on_connect_t, boost::system::error_code ec, boost::asio::ip::tcp::endpoint) -> void
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
tunnel_op<TunnelHandler>::operator()(boost::system::error_code ec,
                                     std::size_t               bytes_transferred,
                                     bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  namespace net  = boost::asio;
  namespace http = boost::beast::http;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    while (true) {
      s.parser.emplace(std::piecewise_construct, std::make_tuple(),
                       std::make_tuple(get_allocator()));

      s.response.emplace(std::piecewise_construct, std::make_tuple(get_allocator()),
                         std::make_tuple(get_allocator()));

      std::cout << "reading in client header...\n";

      BOOST_ASIO_CORO_YIELD
      s.server.async_read_header(*s.parser, std::move(*this));

      if (ec) { goto upcall; }

      std::cout << "parsing URI now:" << s.parser->get().target() << "\n";

      s.uri_parts = foxy::parse_uri(s.parser->get().target());

      s.is_authority = s.uri_parts.is_authority();
      s.is_connect   = s.parser->get().method() == http::verb::connect;
      s.is_absolute  = s.uri_parts.is_absolute();
      s.is_http      = s.uri_parts.is_http();

      if (s.is_connect && s.is_authority && !s.parser->keep_alive()) {
        std::cout << "sending error response...\n";

        s.close_tunnel = true;

        s.response->result(http::status::bad_request);
        s.response->body() = "CONNECT semantics require a persistent connection\n\n";
        s.response->prepare_payload();

        BOOST_ASIO_CORO_YIELD
        s.server.async_write(*s.response, std::move(*this));

        if (ec) { goto upcall; }
        break;
      }

      if ((s.is_connect && s.is_authority) || (s.is_absolute && s.is_http)) {
        std::cout << "connecting to: \n" << s.uri_parts.host() << "\n\n";

        BOOST_ASIO_CORO_YIELD
        {
          auto const scheme =
            s.client.stream.is_ssl() ? boost::string_view("https") : boost::string_view("http");

          auto port = s.uri_parts.port().size() == 0 ? static_cast<std::string>(scheme)
                                                     : static_cast<std::string>(s.uri_parts.port());

          s.client.async_connect(static_cast<std::string>(s.uri_parts.host()), std::move(port),
                                 bind_handler(std::move(*this), on_connect_t{}, _1, _2));
        }

        if (ec) {
          s.response->result(http::status::bad_request);
          s.response->body() =
            "Unable to connect to the remote at: " + static_cast<std::string>(s.uri_parts.host()) +
            "\nError code: " + ec.message() + "\n\n";

          s.response->prepare_payload();

          BOOST_ASIO_CORO_YIELD
          s.server.async_write(*s.response, std::move(*this));

          if (ec) { goto upcall; }
          if (s.parser->get().keep_alive()) { continue; }
          break;
        }

      } else {
        std::cout << "sending malformed response message\n";

        s.response->result(http::status::bad_request);
        s.response->body() =
          "Malformed client request. Use either CONNECT <authority-uri> or <verb> <absolute-uri>";
        s.response->prepare_payload();

        BOOST_ASIO_CORO_YIELD
        s.server.async_write(*s.response, std::move(*this));

        if (ec) { goto upcall; }
        if (s.parser.get().keep_alive()) { continue; }
        break;
      }

      if (s.is_absolute && s.is_http) {
        s.parser->get().keep_alive(false);

        BOOST_ASIO_CORO_YIELD
        {
          auto target =
            s.uri_parts.path().size() == 0
              ? (s.parser->get().method() == http::verb::options ? boost::string_view("*")
                                                                 : boost::string_view("/"))
              : s.uri_parts.path();

          s.parser->get().target(target);

          async_relay(s.server, s.client, std::move(*s.parser),
                      bind_handler(std::move(*this), on_relay_t{}, _1, _2));
        }

        if (ec) { goto upcall; }
        s.close_tunnel = true;
        break;
      }

      std::cout << "going to write back the tunnel response now\n";

      s.response->result(http::status::ok);

      BOOST_ASIO_CORO_YIELD
      s.server.async_write(*s.response, std::move(*this));

      if (ec) {
        s.close_tunnel = true;
        goto upcall;
      }

      s.close_tunnel = false;
      break;
    }

    {
      auto const guard        = std::move(s.work);
      auto const close_tunnel = s.close_tunnel;
      return p_.invoke(boost::system::error_code(), close_tunnel);
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      net::post(bind_handler(std::move(*this), ec, 0));
    }
    auto const work         = std::move(s.work);
    auto const close_tunnel = s.close_tunnel;
    p_.invoke(ec, close_tunnel);
  }
}

template <class TunnelHandler>
auto
async_tunnel(foxy::server_session& server, foxy::client_session& client, TunnelHandler&& handler)
  -> foxy::return_t<TunnelHandler, void(boost::system::error_code, bool)>
{
  boost::asio::async_completion<TunnelHandler, void(boost::system::error_code, bool)> init(handler);

  tunnel_op<foxy::completion_handler_t<TunnelHandler, void(boost::system::error_code, bool)>>(
    server, client, std::move(init.completion_handler))({}, 0, false);

  return init.result.get();
}

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_TUNNEL_HPP_
