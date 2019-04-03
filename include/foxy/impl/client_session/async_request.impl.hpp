//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_

#include <foxy/client_session.hpp>

namespace foxy
{
namespace detail
{
template <class Request, class ResponseParser, class Handler>
struct request_op
  : boost::beast::async_base<Handler, decltype(std::declval<::foxy::session&>().get_executor())>,
    boost::asio::coroutine

{
  ::foxy::session& session;
  Request&         request;
  ResponseParser&  parser;

  request_op()                  = delete;
  request_op(request_op const&) = default;
  request_op(request_op&&)      = default;

  request_op(::foxy::session& session_, Handler handler, Request& request_, ResponseParser& parser_)
    : boost::beast::async_base<Handler, decltype(std::declval<::foxy::session&>().get_executor())>(
        std::move(handler),
        session_.get_executor())
    , session(session_)
    , request(request_)
    , parser(parser_)
  {
    (*this)({}, 0, false);
  }

  auto
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred,
             bool const                is_continuation = true) -> void
  {
    namespace http = boost::beast::http;

    BOOST_ASIO_CORO_REENTER(*this)
    {
      BOOST_ASIO_CORO_YIELD
      http::async_write(session.stream, request, std::move(*this));
      if (ec) { goto upcall; }

      BOOST_ASIO_CORO_YIELD
      http::async_read(session.stream, session.buffer, parser, std::move(*this));
      if (ec) { goto upcall; }

    upcall:
      this->complete(is_continuation, ec);
    }
  }
};

} // namespace detail

template <class Request, class ResponseParser, class RequestHandler>
auto
client_session::async_request(Request&         request,
                              ResponseParser&  parser,
                              RequestHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<RequestHandler>,
                                     void(boost::system::error_code)>::return_type
{
  boost::asio::async_completion<RequestHandler, void(boost::system::error_code)> init(handler);

  return ::foxy::detail::timer_wrap<
    boost::mp11::mp_bind_front<::foxy::detail::request_op, Request, ResponseParser>::template fn>(
    *this, init, request, parser);
}

} // namespace foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_
