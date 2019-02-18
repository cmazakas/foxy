//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_RELAY_HPP_
#define FOXY_DETAIL_RELAY_HPP_

#include <foxy/session.hpp>
#include <foxy/type_traits.hpp>
#include <foxy/detail/export_connect_fields.hpp>
#include <foxy/detail/has_token.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/error.hpp>

#include <boost/system/error_code.hpp>

#include <array>
#include <iostream>

namespace foxy
{
namespace detail
{
template <class Stream, class RelayHandler>
struct relay_op : boost::asio::coroutine
{
public:
  using executor_type = boost::asio::associated_executor_t<
    RelayHandler,
    decltype((std::declval<::foxy::basic_session<Stream>&>().get_executor()))>;

  using allocator_type = boost::asio::associated_allocator_t<RelayHandler>;

  template <bool isRequest, class Body, class Allocator>
  using parser = boost::beast::http::parser<isRequest, Body, Allocator>;

  template <bool isRequest, class Body, class Fields>
  using serializer = boost::beast::http::serializer<isRequest, Body, Fields>;

  using buffer_body = boost::beast::http::buffer_body;
  using empty_body  = boost::beast::http::empty_body;
  using fields      = boost::beast::http::basic_fields<allocator_type>;

  using request  = boost::beast::http::request<buffer_body, fields>;
  using response = boost::beast::http::response<buffer_body, fields>;

private:
  struct state
  {
    std::array<char, 2048> buffer;

    ::foxy::basic_session<Stream>& server;
    ::foxy::basic_session<Stream>& client;

    parser<true, buffer_body, allocator_type> req_parser;
    serializer<true, buffer_body, fields>     req_sr;
    fields                                    req_fields;
    request&                                  req;

    parser<false, buffer_body, allocator_type> res_parser;
    serializer<false, buffer_body, fields>     res_sr;
    fields                                     res_fields;
    response&                                  res;

    boost::system::error_code ec;

    bool close_tunnel;

    boost::asio::executor_work_guard<decltype(server.get_executor())> work;

    explicit state(RelayHandler const&            handler,
                   ::foxy::basic_session<Stream>& server_,
                   ::foxy::basic_session<Stream>& client_)
      : server(server_)
      , client(client_)
      , req_parser(std::piecewise_construct,
                   std::make_tuple(),
                   std::make_tuple(boost::asio::get_associated_allocator(handler)))
      , req_sr(req_parser.get())
      , req_fields(boost::asio::get_associated_allocator(handler))
      , req(req_parser.get())
      , res_parser(std::piecewise_construct,
                   std::make_tuple(),
                   std::make_tuple(boost::asio::get_associated_allocator(handler)))
      , res_sr(res_parser.get())
      , res_fields(boost::asio::get_associated_allocator(handler))
      , res(res_parser.get())
      , close_tunnel{false}
      , work(server.get_executor())
    {
    }

    explicit state(RelayHandler const&                        handler,
                   ::foxy::basic_session<Stream>&             server_,
                   ::foxy::basic_session<Stream>&             client_,
                   parser<true, empty_body, allocator_type>&& req_parser_)
      : server(server_)
      , client(client_)
      , req_parser(std::move(req_parser_))
      , req_sr(req_parser.get())
      , req_fields(boost::asio::get_associated_allocator(handler))
      , req(req_parser.get())
      , res_parser(std::piecewise_construct,
                   std::make_tuple(),
                   std::make_tuple(boost::asio::get_associated_allocator(handler)))
      , res_sr(res_parser.get())
      , res_fields(boost::asio::get_associated_allocator(handler))
      , res(res_parser.get())
      , close_tunnel{false}
      , work(server.get_executor())
    {
    }
  };

  boost::beast::handler_ptr<state, RelayHandler> p_;

public:
  relay_op()                = delete;
  relay_op(relay_op const&) = default;
  relay_op(relay_op&&)      = default;

  template <class DeducedHandler>
  relay_op(::foxy::basic_session<Stream>& server,
           ::foxy::basic_session<Stream>& client,
           DeducedHandler&&               handler)
    : p_(std::forward<DeducedHandler>(handler), server, client)
  {
  }

  template <class DeducedHandler>
  relay_op(::foxy::basic_session<Stream>&             server,
           ::foxy::basic_session<Stream>&             client,
           parser<true, empty_body, allocator_type>&& req_parser,
           DeducedHandler&&                           handler)
    : p_(std::forward<DeducedHandler>(handler), server, client, std::move(req_parser))
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

  auto
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred,
             bool const                is_continuation = true) -> void;
};

template <class Stream, class RelayHandler>
auto
relay_op<Stream, RelayHandler>::operator()(boost::system::error_code ec,
                                           std::size_t const         bytes_transferred,
                                           bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  namespace http = boost::beast::http;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    if (!s.req_parser.is_header_done()) {
      BOOST_ASIO_CORO_YIELD
      s.server.async_read_header(s.req_parser, std::move(*this));
      if (ec) { goto upcall; }
    }

    // remove hop-by-hop headers here and then store them externally...
    // however, if the user is writing a Connection: close, they are communicating to our proxy that
    // they wish to terminate the connection and we want to empower users to gracefully close their
    // server sessions so we propagate the Connection: close field to convey to the remote that we
    // won't be needing to persist the connection beyond this current response cycle
    //
    BOOST_ASIO_CORO_YIELD
    {
      s.close_tunnel        = s.close_tunnel || !s.req.keep_alive();
      auto const is_chunked = s.req.chunked();

      if (::foxy::detail::has_foxy_via(s.req)) { goto upcall; }

      ::foxy::detail::export_connect_fields<fields>(s.req, s.req_fields);

      if (s.close_tunnel) { s.req.keep_alive(false); }
      if (is_chunked) { s.req.chunked(true); }

      s.req.insert(http::field::via, "1.1 foxy");

      s.client.async_write_header(s.req_sr, std::move(*this));
    }
    if (ec) { goto upcall; }

    do {
      s.ec = {};

      if (!s.req_parser.is_done()) {
        s.req.body().data = s.buffer.data();
        s.req.body().size = s.buffer.size();

        BOOST_ASIO_CORO_YIELD
        s.server.async_read(s.req_parser, std::move(*this));

        if (ec == http::error::need_buffer) { ec = {}; }
        if (ec) { s.ec = ec; }

        s.req.body().size = s.buffer.size() - s.req.body().size;
        s.req.body().data = s.buffer.data();
        s.req.body().more = !s.req_parser.is_done();

      } else {
        s.req.body().data = nullptr;
        s.req.body().size = 0;
      }

      BOOST_ASIO_CORO_YIELD
      s.client.async_write(s.req_sr, std::move(*this));

      if (ec == http::error::need_buffer) { ec = {}; }
      if (ec || s.ec) { goto upcall; }

    } while (!s.req_parser.is_done() && !s.req_sr.is_done());

    // TODO: if there's an actual here when reading the response header, send a 502 back to the
    // client; once the header is sent, it doesn't make sense to send a 502 on top of the already
    // serialized header
    //
    BOOST_ASIO_CORO_YIELD
    s.client.async_read_header(s.res_parser, std::move(*this));
    if (ec) { goto upcall; }

    BOOST_ASIO_CORO_YIELD
    {
      s.close_tunnel = s.close_tunnel || !s.res.keep_alive();

      if (::foxy::detail::has_foxy_via(s.res)) { goto upcall; }

      auto const is_chunked = s.res.chunked();

      ::foxy::detail::export_connect_fields<fields>(s.res, s.res_fields);

      if (s.close_tunnel) { s.res.keep_alive(false); }
      if (is_chunked) { s.res.chunked(true); }

      s.res.insert(http::field::via, "1.1 foxy");

      s.server.async_write_header(s.res_sr, std::move(*this));
    }
    if (ec) { goto upcall; }

    do {
      s.ec = {};

      if (!s.res_parser.is_done()) {
        s.res.body().data = s.buffer.data();
        s.res.body().size = s.buffer.size();

        BOOST_ASIO_CORO_YIELD
        s.client.async_read(s.res_parser, std::move(*this));

        if (ec == http::error::need_buffer) { ec = {}; }
        if (ec) { s.ec = ec; }

        s.res.body().size = s.buffer.size() - s.res.body().size;
        s.res.body().data = s.buffer.data();
        s.res.body().more = !s.res_parser.is_done();

      } else {
        s.res.body().data = nullptr;
        s.res.body().size = 0;
      }

      BOOST_ASIO_CORO_YIELD
      s.server.async_write(s.res_sr, std::move(*this));

      if (ec == http::error::need_buffer) { ec = {}; }
      if (ec || s.ec) { goto upcall; }

    } while (!s.res_parser.is_done() && !s.res_sr.is_done());

    {
      auto       work         = std::move(s.work);
      auto const close_tunnel = s.close_tunnel;
      return p_.invoke(boost::system::error_code(), close_tunnel);
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec, true);
  }
}

template <class Stream, class RelayHandler>
auto
async_relay(::foxy::basic_session<Stream>& server,
            ::foxy::basic_session<Stream>& client,
            RelayHandler&&                 handler) ->
  typename boost::asio::async_result<std::decay_t<RelayHandler>,
                                     void(boost::system::error_code, bool)>::return_type
{
  boost::asio::async_completion<RelayHandler, void(boost::system::error_code, bool)> init(handler);

  relay_op<Stream, typename boost::asio::async_completion<
                     RelayHandler, void(boost::system::error_code, bool)>::completion_handler_type>(
    server, client, std::move(init.completion_handler))({}, 0, false);

  return init.result.get();
}

template <class Stream, class Parser, class RelayHandler>
auto
async_relay(::foxy::basic_session<Stream>& server,
            ::foxy::basic_session<Stream>& client,
            Parser&&                       parser,
            RelayHandler&&                 handler) ->
  typename boost::asio::async_result<std::decay_t<RelayHandler>,
                                     void(boost::system::error_code, bool)>::return_type
{
  boost::asio::async_completion<RelayHandler, void(boost::system::error_code, bool)> init(handler);

  relay_op<Stream, typename boost::asio::async_completion<
                     RelayHandler, void(boost::system::error_code, bool)>::completion_handler_type>(
    server, client, std::move(parser), std::move(init.completion_handler))({}, 0, false);

  return init.result.get();
}

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_RELAY_HPP_
