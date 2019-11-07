//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_SHUTDOWN_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_SHUTDOWN_IMPL_HPP_

#include <foxy/client_session.hpp>

namespace foxy
{
template <class DynamicBuffer>
template <class ShutdownHandler>
auto
basic_client_session<DynamicBuffer>::async_shutdown(ShutdownHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ShutdownHandler>,
                                     void(boost::system::error_code)>::return_type
{
  return ::foxy::detail::async_timer<void(boost::system::error_code)>(
    [self = this, coro = boost::asio::coroutine()](auto& cb, boost::system::error_code ec = {},
                                                   std::size_t bytes_transferrred = 0) mutable {
      auto& s = *self;

      BOOST_ASIO_CORO_REENTER(coro)
      {
        if (s.stream.is_ssl()) {
          BOOST_ASIO_CORO_YIELD s.stream.ssl().async_shutdown(std::move(cb));
          if (ec == boost::asio::ssl::error::stream_truncated) { ec = {}; }
          if (ec) { goto upcall; }
        }

        s.stream.plain().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        s.stream.plain().close(ec);

      upcall:
        return cb.complete(ec);
      }
    },
    *this, std::forward<ShutdownHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_SHUTDOWN_IMPL_HPP_
