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
namespace detail
{
template <class DynamicBuffer, class Handler>
struct detect_op : boost::beast::async_base<
                     Handler,
                     typename ::foxy::basic_session<
                       boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                                        boost::asio::io_context::executor_type>,
                       DynamicBuffer>::executor_type>,
                   boost::asio::coroutine
{
  ::foxy::basic_session<
    boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::io_context::executor_type>,
    DynamicBuffer>& session;

  detect_op()                 = delete;
  detect_op(detect_op const&) = default;
  detect_op(detect_op&&)      = default;

  detect_op(
    ::foxy::basic_session<boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                                           boost::asio::io_context::executor_type>,
                          DynamicBuffer>& session_,
    Handler                               handler)
    : boost::beast::async_base<
        Handler,
        typename ::foxy::basic_session<
          boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                           boost::asio::io_context::executor_type>,
          DynamicBuffer>::executor_type>(std::move(handler), session_.get_executor())
    , session(session_)
  {
    (*this)({}, false, false);
  }

  auto
  operator()(boost::system::error_code ec, bool detected_ssl, bool const is_continuation = true)
    -> void
  {
    BOOST_ASIO_CORO_REENTER(*this)
    {
      BOOST_ASIO_CORO_YIELD
      boost::beast::async_detect_ssl(session.stream, session.buffer, std::move(*this));
      if (ec) { goto upcall; }

      return this->complete(is_continuation, boost::system::error_code{}, detected_ssl);

    upcall:
      this->complete(is_continuation, ec, false);
    }
  }
};
} // namespace detail

template <class DynamicBuffer>
template <class DetectHandler>
auto
basic_server_session<DynamicBuffer>::async_detect_ssl(DetectHandler&& handler) ->
  typename boost::asio::async_result<std::decay_t<DetectHandler>,
                                     void(boost::system::error_code, bool)>::return_type
{
  boost::asio::async_completion<DetectHandler, void(boost::system::error_code, bool)> init(handler);

  return ::foxy::detail::timer_wrap<
    boost::mp11::mp_bind_front<::foxy::detail::detect_op, DynamicBuffer>::template fn>(*this, init);
}

} // namespace foxy

#endif // FOXY_IMPL_SERVER_SESSION_ASYNC_DETECT_SSL_HPP_
