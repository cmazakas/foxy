//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SERVER_SESSION_ASYNC_SHUTDOWN_IMPL_HPP_
#define FOXY_IMPL_SERVER_SESSION_ASYNC_SHUTDOWN_IMPL_HPP_

#include <foxy/server_session.hpp>

namespace foxy
{
template <class DynamicBuffer>
template <class ShutdownHandler>
auto
basic_server_session<DynamicBuffer>::async_shutdown(ShutdownHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ShutdownHandler>,
                                     void(boost::system::error_code)>::return_type
{
  return ::foxy::detail::async_timer<void(boost::system::error_code, std::size_t)>(
    [self = this, coro = boost::asio::coroutine()](auto& cb, boost::system::error_code ec = {},
                                                   std::size_t bytes_transferred = 0) mutable {
      auto& s = *self;
      BOOST_ASIO_CORO_REENTER(coro)
      {
        if (s.stream.is_ssl()) {
          BOOST_ASIO_CORO_YIELD s.stream.ssl().async_shutdown(std::move(cb));
        }

        // http rfc 7230 section 6.6 Tear-down
        // -----------------------------------
        // To avoid the TCP reset problem, servers typically close a connection
        // in stages.  First, the server performs a half-close by closing only
        // the write side of the read/write connection.  The server then
        // continues to read from the connection until it receives a
        // corresponding close by the client, or until the server is reasonably
        // certain that its own TCP stack has received the client's
        // acknowledgement of the packet(s) containing the server's last
        // response.  Finally, the server fully closes the connection.
        //

        s.stream.plain().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);

        BOOST_ASIO_CORO_YIELD s.stream.async_read_some(s.buffer.prepare(1024), std::move(cb));

        s.stream.plain().shutdown(boost::asio::ip::tcp::socket::shutdown_receive, ec);
        s.stream.plain().close(ec);

        return cb.complete(ec, 0);
      }
    },
    *this, std::forward<ShutdownHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_SERVER_SESSION_ASYNC_SHUTDOWN_IMPL_HPP_
