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
namespace detail
{
template <class DynamicBuffer, class Handler>
struct handshake_op : boost::beast::async_base<
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

  handshake_op()                    = delete;
  handshake_op(handshake_op const&) = default;
  handshake_op(handshake_op&&)      = default;

  handshake_op(
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
    (*this)({}, 0, false);
  }

  auto
  operator()(boost::system::error_code ec,
             std::size_t               bytes_transferred = 0,
             bool const                is_continuation   = true) -> void
  {
    BOOST_ASIO_CORO_REENTER(*this)
    {
      // forces all users to pay for a branch
      // also saves new users from not updating their underlying stream type
      //
      if (!session.stream.is_ssl() && session.opts.ssl_ctx) {
        session.stream.upgrade(*session.opts.ssl_ctx);
      }

      if (session.buffer.size() > 0) {
        BOOST_ASIO_CORO_YIELD
        session.stream.ssl().async_handshake(boost::asio::ssl::stream_base::server,
                                             session.buffer.data(), std::move(*this));

        if (ec) { goto upcall; }

        session.buffer.consume(bytes_transferred);

        return this->complete(is_continuation, boost::system::error_code{}, bytes_transferred);
      }

      BOOST_ASIO_CORO_YIELD
      session.stream.ssl().async_handshake(boost::asio::ssl::stream_base::server, std::move(*this));
      if (ec) { goto upcall; }

      return this->complete(is_continuation, boost::system::error_code{}, 0);

    upcall:
      this->complete(is_continuation, ec, 0);
    }
  }
};
} // namespace detail

template <class DynamicBuffer>
template <class HandshakeHandler>
auto
basic_server_session<DynamicBuffer>::async_handshake(HandshakeHandler&& handler) ->
  typename boost::asio::async_result<std::decay_t<HandshakeHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  boost::asio::async_completion<HandshakeHandler, void(boost::system::error_code, std::size_t)>
    init(handler);

  return ::foxy::detail::timer_wrap<
    boost::mp11::mp_bind_front<::foxy::detail::handshake_op, DynamicBuffer>::template fn>(*this,
                                                                                          init);
}

} // namespace foxy

#endif // FOXY_IMPL_SERVER_SESSION_ASYNC_HANDSHAKE_HPP_
