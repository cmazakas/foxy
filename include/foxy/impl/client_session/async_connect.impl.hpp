//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_

#include <foxy/client_session.hpp>

namespace foxy
{
namespace detail
{

template <class ConnectHandler>
struct connect_op : boost::asio::coroutine
{
private:

  struct state
  {
    ::foxy::session&                             session;
    std::string                                  host;
    std::string                                  service;
    boost::asio::ip::tcp::resolver               resolver;
    boost::asio::ip::tcp::resolver::results_type results;
    boost::asio::ip::tcp::endpoint               endpoint;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      ConnectHandler const&   handler,
      ::foxy::session&        session_,
      std::string             host_,
      std::string             service_)
    : session(session_)
    , host(std::move(host_))
    , service(std::move(service_))
    , resolver(session.stream.get_executor().context())
    , work(session.get_executor())
    {
    }
  };

  boost::beast::handler_ptr<state, ConnectHandler> p_;

public:
  connect_op()                  = delete;
  connect_op(connect_op const&) = default;
  connect_op(connect_op&&)      = default;

  template <class DeducedHandler>
  connect_op(
    ::foxy::session& session,
    std::string             host,
    std::string             service,
    DeducedHandler&&        handler)
  : p_(
    std::forward<DeducedHandler>(handler),
    session, std::move(host), std::move(service))
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    ConnectHandler,
    decltype(std::declval<::foxy::session&>().get_executor())
  >;

  using allocator_type = boost::asio::associated_allocator_t<ConnectHandler>;

  auto get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
  }

  auto get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  struct on_resolve_t {};
  struct on_connect_t {};

  auto operator()(
    on_resolve_t,
    boost::system::error_code                    ec,
    boost::asio::ip::tcp::resolver::results_type results) -> void;

  auto operator()(
    on_connect_t,
    boost::system::error_code      ec,
    boost::asio::ip::tcp::endpoint endpoint) -> void;

  auto operator()(
    boost::system::error_code ec,
    std::size_t const         bytes_transferred,
    bool const                is_continuation = true) -> void;
};

template <class ConnectHandler>
auto
connect_op<ConnectHandler>::
operator()(
  on_resolve_t,
  boost::system::error_code                    ec,
  boost::asio::ip::tcp::resolver::results_type results) -> void
{
  p_->results = std::move(results);
  (*this)(ec, 0);
}

template <class ConnectHandler>
auto
connect_op<ConnectHandler>::
operator()(
  on_connect_t,
  boost::system::error_code      ec,
  boost::asio::ip::tcp::endpoint endpoint) -> void
{
  p_->endpoint = std::move(endpoint);
  (*this)(ec, 0);
}

template <class ConnectHandler>
auto
connect_op<ConnectHandler>::
operator()(
  boost::system::error_code ec,
  std::size_t const         bytes_transferred,
  bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    if (s.session.stream.is_ssl()) {
      if (!SSL_set_tlsext_host_name(s.session.stream.ssl().native_handle(), s.host.c_str())) {
          ec.assign(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
          goto upcall;
      }
    }

    BOOST_ASIO_CORO_YIELD
    s.resolver.async_resolve(
      s.host, s.service, bind_handler(std::move(*this), on_resolve_t{}, _1, _2));

    if (ec) { goto upcall; }

    BOOST_ASIO_CORO_YIELD
    boost::asio::async_connect(
      s.session.stream.plain(), s.results,
      bind_handler(std::move(*this), on_connect_t{}, _1, _2));

    if (ec) { goto upcall; }

    if (s.session.stream.is_ssl()) {
      BOOST_ASIO_CORO_YIELD
      s.session.stream.ssl().async_handshake(
        boost::asio::ssl::stream_base::client,
        bind_handler(std::move(*this), _1, 0));

      if (ec) { goto upcall; }
    }

    {
      auto endpoint = std::move(s.endpoint);
      auto work     = std::move(s.work);
      return p_.invoke(boost::system::error_code(), std::move(endpoint));
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(boost::beast::bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec, boost::asio::ip::tcp::endpoint());
  }
}

} // detail

template <class ConnectHandler>
auto
client_session::async_connect(
  std::string      host,
  std::string      service,
  ConnectHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(
  ConnectHandler, void(boost::system::error_code, boost::asio::ip::tcp::endpoint))
{
  boost::asio::async_completion<
    ConnectHandler, void(boost::system::error_code, boost::asio::ip::tcp::endpoint)
  >
  init(handler);

  detail::timed_op_wrapper<
    boost::asio::ip::tcp::socket,
    detail::connect_op,
    BOOST_ASIO_HANDLER_TYPE(
      ConnectHandler,
      void(boost::system::error_code, boost::asio::ip::tcp::endpoint)),
    void(boost::system::error_code, boost::asio::ip::tcp::endpoint)
  >(*this, std::move(init.completion_handler)).init(std::move(host), std::move(service));

  return init.result.get();
}

} // foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
