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
template <class DynamicBuffer>
template <class Request, class ResponseParser, class RequestHandler>
auto
basic_client_session<DynamicBuffer>::async_request(Request const&   request,
                                                   ResponseParser&  parser,
                                                   RequestHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<RequestHandler>,
                                     void(boost::system::error_code)>::return_type
{
  return ::foxy::detail::async_timer<void(boost::system::error_code)>(
    [&request, &parser, self = this, coro = boost::asio::coroutine()](
      auto& cb, boost::system::error_code ec = {}, std::size_t bytes_transferrred = 0) mutable {
      auto& s = *self;

      BOOST_ASIO_CORO_REENTER(coro)
      {
        BOOST_ASIO_CORO_YIELD
        boost::beast::http::async_write(s.stream, request, std::move(cb));
        if (ec) { goto upcall; }

        BOOST_ASIO_CORO_YIELD
        boost::beast::http::async_read(s.stream, s.buffer, parser, std::move(cb));
        if (ec) { goto upcall; }

      upcall:
        cb.complete(ec);
      }
    },
    *this, std::forward<RequestHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_
