//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_TUNNEL_HPP_
#define FOXY_DETAIL_TUNNEL_HPP_

#include <foxy/session.hpp>
#include <foxy/uri_parts.hpp>
#include <boost/beast/http/empty_body.hpp>

namespace foxy
{
namespace detail
{
template <class Stream, class TunnelHandler>
struct tunnel_op : boost::asio::coroutine
{
public:
  using body_type = boost::beast::http::empty_body;

  template <bool isRequest, class Body>
  using parser_type = boost::beast::http::parser<isRequest, Body>;

  template <bool isRequest, class Body>
  using serializer_type = boost::beast::http::serializer<isRequest, Body>;

  using fields_type = boost::beast::http::fields;

private:
  struct state
  {
    ::foxy::basic_session<Stream>& server;
    ::foxy::basic_session<Stream>& client;

    parser_type<true, body_type>     req_parser;
    serializer_type<true, body_type> req_sr;
    fields_type                      req_fields;

    parser_type<false, body_type>     res_parser;
    serializer_type<false, body_type> res_sr;
    fields_type                       res_fields;

    // TODO: remember if we need this or not...
    //
    bool close_tunnel;

    boost::asio::executor_work_guard<decltype(server.get_executor())> work;

    explicit state(TunnelHandler const&           handler,
                   ::foxy::basic_session<Stream>& server_,
                   ::foxy::basic_session<Stream>& client_)
      : server(server_)
      , client(client_)
      , req_sr(req_parser.get())
      , res_sr(res_parser.get())
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
  tunnel_op(::foxy::basic_session<Stream>& server,
            ::foxy::basic_session<Stream>& client,
            DeducedHandler&&               handler)
    : p_(std::forward<DeducedHandler>(handler), server, client)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    TunnelHandler,
    decltype((std::declval<::foxy::basic_session<Stream>&>().get_executor()))>;

  using allocator_type = boost::asio::associated_allocator_t<TunnelHandler>;

  auto
  get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(),
                                                p_->server.get_executor());
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

template <class Stream, class TunnelHandler>
auto
tunnel_op<Stream, TunnelHandler>::
operator()(boost::system::error_code ec,
           std::size_t const         bytes_transferred,
           bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  namespace http = boost::beast::http;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    BOOST_ASIO_CORO_YIELD
    s.server.async_read_header(s.req_parser, std::move(*this));

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec, false);
  }
}

template <class Stream, class TunnelHandler>
auto
async_tunnel(::foxy::basic_session<Stream>& server,
             ::foxy::basic_session<Stream>& client,
             TunnelHandler&&                handler)
  -> BOOST_ASIO_INITFN_RESULT_TYPE(TunnelHandler,
                                   void(boost::system::error_code, bool))
{
  boost::asio::async_completion<TunnelHandler,
                                void(boost::system::error_code, bool)>
    init(handler);

  tunnel_op<Stream, BOOST_ASIO_HANDLER_TYPE(
                      TunnelHandler, void(boost::system::error_code, bool))>(
    server, client, std::move(init.completion_handler))({}, 0, false);

  return init.result.get();
}

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_TUNNEL_HPP_
