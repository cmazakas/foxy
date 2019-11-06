//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SERVER_SESSION_ASYNC_DETECT_SSL_HPP_
#define FOXY_IMPL_SERVER_SESSION_ASYNC_DETECT_SSL_HPP_

#include <foxy/server_session.hpp>

namespace foxy
{
template <class DynamicBuffer>
template <class DetectHandler>
auto
basic_server_session<DynamicBuffer>::async_detect_ssl(DetectHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<DetectHandler>,
                                     void(boost::system::error_code, bool)>::return_type
{
  return ::foxy::detail::async_timer<void(boost::system::error_code, bool)>(
    [self = this, coro = boost::asio::coroutine()](auto& cb, boost::system::error_code ec = {},
                                                   bool detected_ssl = false) mutable {
      auto& s = *self;
      BOOST_ASIO_CORO_REENTER(coro)
      {
        BOOST_ASIO_CORO_YIELD boost::beast::async_detect_ssl(s.stream, s.buffer, std::move(cb));
        return cb.complete(ec, detected_ssl);
      }
    },
    *this, std::forward<DetectHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_SERVER_SESSION_ASYNC_DETECT_SSL_HPP_
