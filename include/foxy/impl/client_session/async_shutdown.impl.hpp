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
namespace detail
{
template <class DynamicBuffer, class Handler>
struct shutdown_op
  : boost::beast::async_base<
      Handler,
      typename ::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>::executor_type>,
    boost::asio::coroutine

{
  ::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>& session;

  shutdown_op()                   = delete;
  shutdown_op(shutdown_op const&) = default;
  shutdown_op(shutdown_op&&)      = default;

  shutdown_op(::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>& session_,
              Handler                                                             handler)
    : boost::beast::async_base<
        Handler,
        typename ::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>::executor_type>(
        std::move(handler),
        session_.get_executor())
    , session(session_)
  {
    (*this)({}, 0, false);
  }

  auto
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred = 0,
             bool const                is_continuation   = true) -> void
  {
    namespace http = boost::beast::http;

    BOOST_ASIO_CORO_REENTER(*this)
    {
      if (session.stream.is_ssl()) {
        BOOST_ASIO_CORO_YIELD session.stream.ssl().async_shutdown(std::move(*this));
        if (ec) { goto upcall; }

        session.stream.ssl().next_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        session.stream.ssl().next_layer().close(ec);
      } else {
        session.stream.plain().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
        session.stream.plain().close(ec);
      }

    upcall:
      this->complete(is_continuation, ec);
    }
  }
};

} // namespace detail

template <class DynamicBuffer>
template <class ShutdownHandler>
auto
basic_client_session<DynamicBuffer>::async_shutdown(ShutdownHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ShutdownHandler>,
                                     void(boost::system::error_code)>::return_type
{
  return ::foxy::detail::timer_initiate<
    void(boost::system::error_code),
    boost::mp11::mp_bind_front<::foxy::detail::shutdown_op, DynamicBuffer>::template fn>(
    *this, std::forward<ShutdownHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_SHUTDOWN_IMPL_HPP_
