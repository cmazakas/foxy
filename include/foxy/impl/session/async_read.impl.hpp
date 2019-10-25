//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_
#define FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_

#include <foxy/session.hpp>
#include <foxy/detail/timed_op_wrapper_v3.hpp>

namespace foxy
{
template <class Stream, class DynamicBuffer>
template <class Parser, class ReadHandler>
auto
basic_session<Stream, DynamicBuffer>::async_read(Parser& parser, ReadHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  return ::foxy::detail::async_timer<void(boost::system::error_code, std::size_t)>(
    [&parser, self = this, coro = boost::asio::coroutine()](
      auto& cb, boost::system::error_code ec = {}, std::size_t bytes_transferrred = 0) mutable {
      BOOST_ASIO_CORO_REENTER(coro)
      {
        BOOST_ASIO_CORO_YIELD boost::beast::http::async_read(self->stream, self->buffer, parser,
                                                             std::move(cb));

        cb.complete(ec, bytes_transferrred);
      }
    },
    *this, std::forward<ReadHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_
