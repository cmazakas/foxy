//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SESSION_ASYNC_WRITE_HEADER_IMPL_HPP_
#define FOXY_IMPL_SESSION_ASYNC_WRITE_HEADER_IMPL_HPP_

#include <foxy/session.hpp>
#include <foxy/detail/timed_op_wrapper_v3.hpp>

namespace foxy
{
template <class Stream, class DynamicBuffer>
template <class Serializer, class WriteHandler>
auto
basic_session<Stream, DynamicBuffer>::async_write_header(Serializer&    serializer,
                                                         WriteHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<WriteHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  return ::foxy::detail::async_timer<void(boost::system::error_code, std::size_t)>(
    [&serializer, self = this, coro = boost::asio::coroutine()](
      auto& cb, boost::system::error_code ec = {}, std::size_t bytes_transferrred = 0) mutable {
      BOOST_ASIO_CORO_REENTER(coro)
      {
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_write_header(self->stream, serializer,
                                                                     std::move(cb));

        cb.complete(ec, bytes_transferrred);
      }
    },
    *this, std::forward<WriteHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_SESSION_ASYNC_WRITE_HEADER_IMPL_HPP_
