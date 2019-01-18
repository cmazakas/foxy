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
    decltype(
      (std::declval<::foxy::basic_session<boost::asio::ip::tcp::socket>&>().get_executor()))>;

  using allocator_type = boost::asio::associated_allocator_t<TunnelHandler>;

private:
  struct state
  {
    ::foxy::server_session& server;
    ::foxy::client_session& client;

    boost::beast::http::request_parser<boost::beast::http::empty_body, allocator_type> parser;
    boost::beast::http::response<boost::beast::http::string_body,
                                 boost::beast::http::basic_fields<allocator_type>>
      err_response;

    ::foxy::uri_parts uri_parts;

    bool close_tunnel;

    boost::asio::executor_work_guard<decltype(server.get_executor())> work;

    explicit state(TunnelHandler const&    handler,
                   ::foxy::server_session& server_,
                   ::foxy::client_session& client_)
      : server(server_)
      , client(client_)
      , parser(std::piecewise_construct,
               std::make_tuple(),
               std::make_tuple(boost::asio::get_associated_allocator(handler)))
      , err_response(std::piecewise_construct,
                     std::make_tuple(),
                     std::make_tuple(boost::asio::get_associated_allocator(handler)))
      , close_tunnel{false}
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
  tunnel_op(::foxy::server_session& server,
            ::foxy::client_session& client,
            DeducedHandler&&        handler)
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
                                     std::size_t const         bytes_transferred,
                                     bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  namespace net  = boost::asio;
  namespace http = boost::beast::http;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    BOOST_ASIO_CORO_YIELD
    s.server.async_read_header(s.parser, std::move(*this));
    if (ec) { goto upcall; }

    BOOST_ASIO_CORO_YIELD
    {
      auto const& request = s.parser.get();

      s.uri_parts = foxy::parse_uri(request.target());

      auto const is_authority = s.uri_parts.is_authority();
      auto const is_absolute  = s.uri_parts.is_absolute();
      auto const is_http      = s.uri_parts.is_http();
      auto const is_connect   = request.method() == http::verb::connect;

      // right now, we're only going to support one-time relays
      // in this case, we only forward messages that send in an absolute URL
      // we take a little extra precaution around the user sending us a funky URI with an incorrect
      // scheme
      //
      if (!is_absolute || !is_http) {
        s.err_response.result(http::status::bad_request);
        s.err_response.body() =
          "Currently only non-persistent proxying is supported.\nUse an absolute URI as your "
          "request target for the proxy.\n";
        s.err_response.prepare_payload();

        s.server.async_write(s.err_response, std::move(*this));

      } else {
        std::cout << "connecting to: \n" << s.uri_parts.host() << "\n\n";

        s.client.async_connect(static_cast<std::string>(s.uri_parts.host()),
                               static_cast<std::string>(s.uri_parts.port()),
                               bind_handler(std::move(*this), on_connect_t{}, _1, _2));
      }
    }

    s.parser.get().keep_alive(false);
    s.parser.get().target(s.uri_parts.path());

    BOOST_ASIO_CORO_YIELD
    async_relay(s.server, s.client, std::move(s.parser),
                bind_handler(std::move(*this), on_relay_t{}, _1, _2));

    if (ec) { goto upcall; }

    {
      auto guard = std::move(s.work);
      return p_.invoke(boost::system::error_code(), true);
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      net::post(bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec, true);
  }
}

template <class TunnelHandler>
auto
async_tunnel(::foxy::server_session& server,
             ::foxy::client_session& client,
             TunnelHandler&&         handler)
  -> ::foxy::return_t<TunnelHandler, void(boost::system::error_code, bool)>
{
  boost::asio::async_completion<TunnelHandler, void(boost::system::error_code, bool)> init(handler);

  tunnel_op<::foxy::completion_handler_t<TunnelHandler, void(boost::system::error_code, bool)>>(
    server, client, std::move(init.completion_handler))({}, 0, false);

  return init.result.get();
}

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_TUNNEL_HPP_
