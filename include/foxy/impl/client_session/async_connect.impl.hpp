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
template <class DynamicBuffer, class Executor, class Allocator>
struct connect_op : boost::asio::coroutine
{
  struct state
  {
    boost::asio::ip::tcp::resolver               resolver;
    boost::asio::ip::tcp::resolver::results_type endpoint_range;
    boost::asio::ip::tcp::endpoint               endpoint;
    std::string                                  host;
    std::string                                  service;
  };

  std::unique_ptr<state, boost::alloc_deleter<state, Allocator>>      p_;
  ::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>& session;

  connect_op()                  = delete;
  connect_op(connect_op const&) = delete;
  connect_op(connect_op&&)      = default;

  connect_op(Allocator const&                                                    allocator,
             Executor                                                            executor,
             std::string                                                         host_,
             std::string                                                         service_,
             ::foxy::basic_session<boost::asio::ip::tcp::socket, DynamicBuffer>& session_)
    : p_(boost::allocate_unique<state>(
        allocator,
        {boost::asio::ip::tcp::resolver(executor), boost::asio::ip::tcp::resolver::results_type{},
         boost::asio::ip::tcp::endpoint{}, std::move(host_), std::move(service_)}))
    , session(session_)
  {
  }

  struct on_resolve_t
  {
  };
  struct on_connect_t
  {
  };

  template <class Self>
  auto
  operator()(Self& self,
             on_resolve_t,
             boost::system::error_code                    ec,
             boost::asio::ip::tcp::resolver::results_type results) -> void
  {
    p_->endpoint_range = std::move(results);
    (*this)(self, ec, 0);
  }

  template <class Self>
  auto
  operator()(Self& self,
             on_connect_t,
             boost::system::error_code      ec,
             boost::asio::ip::tcp::endpoint endpoint_) -> void
  {
    p_->endpoint = std::move(endpoint_);
    (*this)(self, ec, 0);
  }

  template <class Self>
  auto operator()(Self&                     self,
                  boost::system::error_code ec                = {},
                  std::size_t const         bytes_transferred = 0) -> void
  {
    auto& s = *p_;
    BOOST_ASIO_CORO_REENTER(*this)
    {
      BOOST_ASIO_CORO_YIELD s.resolver.async_resolve(
        s.host, s.service, boost::beast::bind_front_handler(std::move(self), on_resolve_t{}));

      if (ec) { goto upcall; }

      BOOST_ASIO_CORO_YIELD boost::asio::async_connect(
        session.stream.plain(), s.endpoint_range,
        boost::beast::bind_front_handler(std::move(self), on_connect_t{}));

      if (ec) { goto upcall; }

      if (session.stream.is_ssl()) {
        if (session.opts.verify_peer_cert) {
          ::foxy::certify::set_sni_hostname(session.stream.ssl(), s.host);
          ::foxy::certify::set_server_hostname(session.stream.ssl().native_handle(), s.host);
        }

        BOOST_ASIO_CORO_YIELD
        session.stream.ssl().async_handshake(boost::asio::ssl::stream_base::client,
                                             std::move(self));

        if (ec) { goto upcall; }
      }

    upcall:
      p_.reset();
      return self.complete(ec);
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
  auto const allocator = boost::asio::get_associated_allocator(handler);
  auto const executor  = boost::asio::get_associated_executor(handler, this->stream.get_executor());

  return ::foxy::detail::async_timer<void(boost::system::error_code)>(
    ::foxy::detail::connect_op<DynamicBuffer, std::decay_t<decltype(executor)>,
                               std::decay_t<decltype(allocator)>>(
      allocator, executor, std::move(host), std::move(service), *this),
    *this, std::forward<ConnectHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
