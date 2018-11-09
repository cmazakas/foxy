//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_RELAY_HPP_
#define FOXY_DETAIL_RELAY_HPP_

#include <foxy/session.hpp>
#include <foxy/detail/export_connect_fields.hpp>

#include <boost/optional/optional.hpp>

#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/error.hpp>

#include <array>
#include <iostream>

namespace foxy
{
namespace detail
{
template <class Stream, class RelayHandler>
struct relay_op : boost::asio::coroutine
{
private:
  struct frame_type
  {
    using body_type = boost::beast::http::buffer_body;

    template <bool isRequest, class Body>
    using parser_type = boost::beast::http::parser<isRequest, Body>;

    template <bool isRequest, class Body>
    using serializer_type = boost::beast::http::serializer<isRequest, Body>;

    using fields_type = boost::beast::http::fields;

    parser_type<true, body_type>     req_parser;
    serializer_type<true, body_type> req_sr;
    fields_type                      req_fields;

    parser_type<false, body_type>     res_parser;
    serializer_type<false, body_type> res_sr;
    fields_type                       res_fields;

    frame_type()
      : req_sr(req_parser.get())
      , res_sr(res_parser.get())
    {
    }

    frame_type(frame_type const&) = default;
    frame_type(frame_type&&)      = default;
  };

  struct state
  {
    boost::optional<frame_type> frame;
    std::array<char, 2048>      buffer;

    ::foxy::basic_session<Stream>& server;
    ::foxy::basic_session<Stream>& client;

    boost::asio::executor_work_guard<decltype(server.get_executor())> work;

    explicit state(RelayHandler const&            handler,
                   ::foxy::basic_session<Stream>& server_,
                   ::foxy::basic_session<Stream>& client_)
      : server(server_)
      , client(client_)
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

  using executor_type = boost::asio::associated_executor_t<
    RelayHandler,
    decltype((std::declval<::foxy::basic_session<Stream>&>().get_executor()))>;

  using allocator_type = boost::asio::associated_allocator_t<RelayHandler>;

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

template <class Stream, class RelayHandler>
auto
relay_op<Stream, RelayHandler>::operator()(boost::system::error_code ec,
                                           std::size_t const bytes_transferred,
                                           bool const is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  namespace http = boost::beast::http;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    s.frame = boost::none;
    s.frame.emplace();

    BOOST_ASIO_CORO_YIELD
    s.server.async_read_header(s.frame->req_parser, std::move(*this));
    if (ec) { goto upcall; }

    // remove hop-by-hop headers here and then store them externally
    //
    ::foxy::detail::export_connect_fields<typename frame_type::fields_type>(
      s.frame->req_parser.get(), s.frame->res_fields);

    BOOST_ASIO_CORO_YIELD
    s.client.async_write_header(s.frame->req_sr, std::move(*this));
    if (ec) { goto upcall; }

    do {
      if (!s.frame->req_parser.is_done()) {
        s.frame->req_parser.get().body().data = s.buffer.data();
        s.frame->req_parser.get().body().size = s.buffer.size();

        BOOST_ASIO_CORO_YIELD
        s.server.async_read(s.frame->req_parser, std::move(*this));
        if (ec == http::error::need_buffer) { ec = {}; }
        if (ec) { goto upcall; }

        s.frame->req_parser.get().body().size =
          s.buffer.size() - s.frame->req_parser.get().body().size;
        s.frame->req_parser.get().body().data = s.buffer.data();
        s.frame->req_parser.get().body().more = !s.frame->req_parser.is_done();

      } else {
        s.frame->req_parser.get().body().data = nullptr;
        s.frame->req_parser.get().body().size = 0;
      }

      BOOST_ASIO_CORO_YIELD
      s.client.async_write(s.frame->req_sr, std::move(*this));
      if (ec == http::error::need_buffer) { ec = {}; }
      if (ec) { goto upcall; }
    } while (!s.frame->req_parser.is_done() && !s.frame->req_sr.is_done());

    BOOST_ASIO_CORO_YIELD
    s.client.async_read_header(s.frame->res_parser, std::move(*this));
    if (ec) { goto upcall; }

    BOOST_ASIO_CORO_YIELD
    s.server.async_write_header(s.frame->res_sr, std::move(*this));
    if (ec) { goto upcall; }

    do {
      if (!s.frame->res_parser.is_done()) {
        s.frame->res_parser.get().body().data = s.buffer.data();
        s.frame->res_parser.get().body().size = s.buffer.size();

        BOOST_ASIO_CORO_YIELD
        s.client.async_read(s.frame->res_parser, std::move(*this));
        if (ec == http::error::need_buffer) { ec = {}; }
        if (ec) { goto upcall; }

        s.frame->res_parser.get().body().size =
          s.buffer.size() - s.frame->res_parser.get().body().size;
        s.frame->res_parser.get().body().data = s.buffer.data();
        s.frame->res_parser.get().body().more = !s.frame->res_parser.is_done();

      } else {
        s.frame->res_parser.get().body().data = nullptr;
        s.frame->res_parser.get().body().size = 0;
      }

      BOOST_ASIO_CORO_YIELD
      s.server.async_write(s.frame->res_sr, std::move(*this));
      if (ec == http::error::need_buffer) { ec = {}; }
      if (ec) { goto upcall; }
    } while (!s.frame->res_parser.is_done() && !s.frame->res_sr.is_done());

    {
      auto work = std::move(s.work);
      return p_.invoke(boost::system::error_code(), bytes_transferred);
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec, 0);
  }
}

template <class Stream, class RelayHandler>
auto
async_relay(::foxy::basic_session<Stream>& server,
            ::foxy::basic_session<Stream>& client,
            RelayHandler&&                 handler)
  -> BOOST_ASIO_INITFN_RESULT_TYPE(RelayHandler,
                                   void(boost::system::error_code, std::size_t))
{
  boost::asio::async_completion<RelayHandler,
                                void(boost::system::error_code, std::size_t)>
    init(handler);

  relay_op<Stream,
           BOOST_ASIO_HANDLER_TYPE(
             RelayHandler, void(boost::system::error_code, std::size_t))>(
    server, client, std::move(init.completion_handler))({}, 0, false);

  return init.result.get();
}

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_RELAY_HPP_
