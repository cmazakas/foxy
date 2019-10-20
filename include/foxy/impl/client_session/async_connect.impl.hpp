//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_

#include <foxy/client_session.hpp>

namespace foxy
{
namespace detail
{
template <class DynamicBuffer, class Handler>
struct connect_op
  : boost::beast::stable_async_base<
      Handler,
      typename ::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>::executor_type>,
    boost::asio::coroutine
{
  struct state
  {
    std::string                                  host;
    std::string                                  service;
    boost::asio::ip::tcp::resolver               resolver;
    boost::asio::ip::tcp::resolver::results_type results;
    boost::asio::ip::tcp::endpoint               endpoint;

    state(boost::asio::executor executor, std::string host_, std::string service_)
      : host(std::move(host_))
      , service(std::move(service_))
      , resolver(executor)
    {
    }
  };

  ::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>& session;
  state&                                                              s;

  connect_op()                  = default;
  connect_op(connect_op const&) = default;
  connect_op(connect_op&&)      = default;

  connect_op(::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>& session_,
             Handler                                                             handler,
             std::string                                                         host,
             std::string                                                         service)
    : boost::beast::stable_async_base<Handler, typename ::foxy::session::executor_type>(
        std::move(handler),
        session_.get_executor())
    , session(session_)
    , s(boost::beast::allocate_stable<state>(*this,
                                             session.get_executor(),
                                             std::move(host),
                                             std::move(service)))
  {
    (*this)({}, 0, false);
  }

  struct on_resolve_t
  {
  };
  struct on_connect_t
  {
  };

  auto
  operator()(on_resolve_t,
             boost::system::error_code                    ec,
             boost::asio::ip::tcp::resolver::results_type results) -> void
  {
    s.results = std::move(results);
    (*this)(ec, 0);
  }

  auto
  operator()(on_connect_t, boost::system::error_code ec, boost::asio::ip::tcp::endpoint endpoint)
    -> void
  {
    s.endpoint = std::move(endpoint);
    (*this)(ec, 0);
  }

  auto
  operator()(boost::system::error_code ec) -> void
  {
    (*this)(ec, 0);
  }

  auto
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred,
             bool const                is_continuation = true) -> void
  {
    using namespace std::placeholders;

    BOOST_ASIO_CORO_REENTER(*this)
    {
      BOOST_ASIO_CORO_YIELD
      s.resolver.async_resolve(s.host, s.service,
                               boost::beast::bind_front_handler(std::move(*this), on_resolve_t{}));

      if (ec) { goto upcall; }

      BOOST_ASIO_CORO_YIELD
      {
        auto& socket =
          session.stream.is_ssl() ? session.stream.ssl().next_layer() : session.stream.plain();

        boost::asio::async_connect(
          socket, s.results, boost::beast::bind_front_handler(std::move(*this), on_connect_t{}));
      }

      if (ec) { goto upcall; }

      if (session.stream.is_ssl()) {
        if (session.opts.verify_peer_cert) {
          ::foxy::certify::set_sni_hostname(session.stream.ssl(), s.host);
          ::foxy::certify::set_server_hostname(session.stream.ssl().native_handle(), s.host);
        }

        BOOST_ASIO_CORO_YIELD
        session.stream.ssl().async_handshake(boost::asio::ssl::stream_base::client,
                                             std::move(*this));

        if (ec) { goto upcall; }
      }

      return this->complete_now(boost::system::error_code{});

    upcall:
      if (!is_continuation) {
        BOOST_ASIO_CORO_YIELD
        boost::asio::post(boost::beast::bind_handler(std::move(*this), ec, 0));
      }

      return this->complete_now(ec);
    }
  }
};
} // namespace detail

template <class DynamicBuffer>
template <class ConnectHandler>
auto
basic_client_session<DynamicBuffer>::async_connect(std::string      host,
                                                   std::string      service,
                                                   ConnectHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ConnectHandler>,
                                     void(boost::system::error_code)>::return_type
{
  return ::foxy::detail::timer_initiate<
    void(boost::system::error_code),
    boost::mp11::mp_bind_front<::foxy::detail::connect_op, DynamicBuffer>::template fn>(
    *this, std::forward<ConnectHandler>(handler), std::move(host), std::move(service));
}

} // namespace foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
