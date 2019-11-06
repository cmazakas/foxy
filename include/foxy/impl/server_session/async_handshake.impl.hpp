//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SERVER_SESSION_ASYNC_HANDSHAKE_HPP_
#define FOXY_IMPL_SERVER_SESSION_ASYNC_HANDSHAKE_HPP_

#include <foxy/server_session.hpp>

namespace foxy
{
template <class DynamicBuffer>
template <class HandshakeHandler>
auto
basic_server_session<DynamicBuffer>::async_handshake(HandshakeHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<HandshakeHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  return ::foxy::detail::async_timer<void(boost::system::error_code, std::size_t)>(
    [self = this, coro = boost::asio::coroutine()](auto& cb, boost::system::error_code ec = {},
                                                   std::size_t bytes_transferred = 0) mutable {
      auto& s = *self;
      BOOST_ASIO_CORO_REENTER(coro)
      {
        if (!s.stream.is_ssl() && s.opts.ssl_ctx) { s.stream.upgrade(*s.opts.ssl_ctx); }

        if (s.buffer.size() > 0) {
          BOOST_ASIO_CORO_YIELD
          s.stream.ssl().async_handshake(boost::asio::ssl::stream_base::server, s.buffer.data(),
                                         std::move(cb));
          if (ec) { goto upcall; }

          s.buffer.consume(bytes_transferred);

          return cb.complete(boost::system::error_code{}, bytes_transferred);
        }

        BOOST_ASIO_CORO_YIELD
        s.stream.ssl().async_handshake(boost::asio::ssl::stream_base::server, std::move(cb));
        if (ec) { goto upcall; }

        return cb.complete(boost::system::error_code{}, 0);

      upcall:
        return cb.complete(ec, 0);
      }
    },
    *this, std::forward<HandshakeHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_SERVER_SESSION_ASYNC_HANDSHAKE_HPP_
