//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_

#include <foxy/client_session.hpp>
#include <foxy/type_traits.hpp>
#include <boost/beast/core/async_base.hpp>

#include <iostream>

namespace foxy
{
namespace detail
{
// template <class ConnectHandler>
// struct connect_op : boost::asio::coroutine
// {
// private:
//   struct state
//   {
//     ::foxy::session&                             session;
//     std::string                                  host;
//     std::string                                  service;
//     boost::asio::ip::tcp::resolver               resolver;
//     boost::asio::ip::tcp::resolver::results_type results;
//     boost::asio::ip::tcp::endpoint               endpoint;

//     boost::asio::executor_work_guard<decltype(session.get_executor())> work;

//     explicit state(ConnectHandler const& handler,
//                    ::foxy::session&      session_,
//                    std::string           host_,
//                    std::string           service_)
//       : session(session_)
//       , host(std::move(host_))
//       , service(std::move(service_))
//       , resolver(session.stream.get_executor().context())
//       , work(session.get_executor())
//     {
//     }
//   };

//   boost::beast::handler_ptr<state, ConnectHandler> p_;

// public:
//   connect_op()                  = delete;
//   connect_op(connect_op const&) = default;
//   connect_op(connect_op&&)      = default;

//   template <class DeducedHandler>
//   connect_op(::foxy::session& session,
//              DeducedHandler&& handler,
//              std::string      host,
//              std::string      service)
//     : p_(std::forward<DeducedHandler>(handler), session, std::move(host), std::move(service))
//   {
//     (*this)({}, 0, false);
//   }

//   using executor_type =
//     boost::asio::associated_executor_t<ConnectHandler,
//                                        decltype(std::declval<::foxy::session&>().get_executor())>;

//   using allocator_type = boost::asio::associated_allocator_t<ConnectHandler>;

//   auto
//   get_executor() const noexcept -> executor_type
//   {
//     return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
//   }

//   auto
//   get_allocator() const noexcept -> allocator_type
//   {
//     return boost::asio::get_associated_allocator(p_.handler());
//   }

//   struct on_resolve_t
//   {
//   };
//   struct on_connect_t
//   {
//   };

//   auto
//   operator()(on_resolve_t,
//              boost::system::error_code                    ec,
//              boost::asio::ip::tcp::resolver::results_type results) -> void;

//   auto
//   operator()(on_connect_t, boost::system::error_code ec, boost::asio::ip::tcp::endpoint endpoint)
//     -> void;

//   auto
//   operator()(boost::system::error_code ec,
//              std::size_t const         bytes_transferred,
//              bool const                is_continuation = true) -> void;
// };

// template <class ConnectHandler>
// auto
// connect_op<ConnectHandler>::operator()(on_resolve_t,
//                                        boost::system::error_code                    ec,
//                                        boost::asio::ip::tcp::resolver::results_type results) ->
//                                        void
// {
//   p_->results = std::move(results);
//   (*this)(ec, 0);
// }

// template <class ConnectHandler>
// auto
// connect_op<ConnectHandler>::operator()(on_connect_t,
//                                        boost::system::error_code      ec,
//                                        boost::asio::ip::tcp::endpoint endpoint) -> void
// {
//   p_->endpoint = std::move(endpoint);
//   (*this)(ec, 0);
// }

// template <class ConnectHandler>
// auto
// connect_op<ConnectHandler>::operator()(boost::system::error_code ec,
//                                        std::size_t const         bytes_transferred,
//                                        bool const                is_continuation) -> void
// {
//   using namespace std::placeholders;
//   using boost::beast::bind_handler;

//   auto& s = *p_;
//   BOOST_ASIO_CORO_REENTER(*this)
//   {
//     if (s.session.stream.is_ssl()) {
//       if (!SSL_set_tlsext_host_name(s.session.stream.ssl().native_handle(), s.host.c_str())) {
//         ec.assign(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
//         goto upcall;
//       }
//     }

//     BOOST_ASIO_CORO_YIELD
//     s.resolver.async_resolve(s.host, s.service,
//                              bind_handler(std::move(*this), on_resolve_t{}, _1, _2));

//     if (ec) { goto upcall; }

//     BOOST_ASIO_CORO_YIELD
//     {
//       auto& socket =
//         s.session.stream.is_ssl() ? s.session.stream.ssl().next_layer() :
//         s.session.stream.plain();
//       boost::asio::async_connect(socket, s.results,
//                                  bind_handler(std::move(*this), on_connect_t{}, _1, _2));
//     }

//     if (ec) { goto upcall; }

//     if (s.session.stream.is_ssl()) {
//       BOOST_ASIO_CORO_YIELD
//       s.session.stream.ssl().async_handshake(boost::asio::ssl::stream_base::client,
//                                              bind_handler(std::move(*this), _1, 0));

//       if (ec) { goto upcall; }
//     }

//     {
//       auto endpoint = std::move(s.endpoint);
//       auto work     = std::move(s.work);
//       return p_.invoke(boost::system::error_code{}, std::move(endpoint));
//     }

//   upcall:
//     if (!is_continuation) {
//       BOOST_ASIO_CORO_YIELD
//       boost::asio::post(boost::beast::bind_handler(std::move(*this), ec, 0));
//     }

//     auto work = std::move(s.work);
//     return p_.invoke(ec, boost::asio::ip::tcp::endpoint{});
//   }
// }

template <class Handler>
struct connect_op
  : boost::beast::stable_async_base<Handler,
                                    boost::asio::associated_executor_t<
                                      Handler,
                                      decltype(std::declval<::foxy::session&>().get_executor())>>,
    boost::asio::coroutine
{
  struct state
  {
    std::string                                  host;
    std::string                                  service;
    boost::asio::ip::tcp::resolver               resolver;
    boost::asio::ip::tcp::resolver::results_type results;
    boost::asio::ip::tcp::endpoint               endpoint;

    explicit state(boost::asio::io_context& io, std::string host_, std::string service_)
      : host(std::move(host_))
      , service(std::move(service_))
      , resolver(io)
    {
    }
  };

  ::foxy::session& session;
  state&           s;

  connect_op(::foxy::session& session_, Handler& handler, std::string host, std::string service)
    : boost::beast::stable_async_base<Handler,
                                      boost::asio::associated_executor_t<
                                        Handler,
                                        decltype(std::declval<::foxy::session&>().get_executor())>>(
        std::move(handler),
        session_.get_executor())
    , session(session_)
    , s(boost::beast::allocate_stable<state>(*this,
                                             session.get_executor().context(),
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
    std::cout << "in on_resolve_t hook...\n";
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
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred,
             bool const                is_continuation = true) -> void
  {
    using namespace std::placeholders;

    BOOST_ASIO_CORO_REENTER(*this)
    {
      std::cout << "beginning connect op...\n";

      if (session.stream.is_ssl()) {
        if (!SSL_set_tlsext_host_name(session.stream.ssl().native_handle(), s.host.c_str())) {
          ec.assign(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
          goto upcall;
        }
      }

      std::cout << "calling async resolve (" << s.host << ", " << s.service << ")...\n";
      // s.results = s.resolver.resolve(s.host, s.service, ec);
      BOOST_ASIO_CORO_YIELD
      s.resolver.async_resolve(s.host, s.service,
                               boost::beast::bind_front_handler(std::move(*this), on_resolve_t{}));

      if (ec) { goto upcall; }

      std::cout << "resolved successfully...\nConnecting to remote now...\n";

      BOOST_ASIO_CORO_YIELD
      {
        auto& socket =
          session.stream.is_ssl() ? session.stream.ssl().next_layer() : session.stream.plain();
        boost::asio::async_connect(
          socket, s.results, boost::beast::bind_front_handler(std::move(*this), on_connect_t{}));
      }

      if (ec) { goto upcall; }

      std::cout << "connected successfully...\n";
      if (session.stream.is_ssl()) {
        BOOST_ASIO_CORO_YIELD
        session.stream.ssl().async_handshake(boost::asio::ssl::stream_base::client,
                                             boost::beast::bind_handler(std::move(*this), _1, 0));

        if (ec) { goto upcall; }
      }

      std::cout << "invoking final completion handler...\n";
      {
        auto endpoint = std::move(s.endpoint);
        return this->complete_now(boost::system::error_code{}, std::move(endpoint));
      }

    upcall:
      if (!is_continuation) {
        BOOST_ASIO_CORO_YIELD
        boost::asio::post(boost::beast::bind_handler(std::move(*this), ec, 0));
      }

      std::cout << "encountered an error : " << ec.message() << "\n";
      return this->complete_now(ec, boost::asio::ip::tcp::endpoint{});
    }
  }
};
} // namespace detail

template <class ConnectHandler>
auto
client_session::async_connect(std::string host, std::string service, ConnectHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ConnectHandler>,
                                     void(boost::system::error_code,
                                          boost::asio::ip::tcp::endpoint)>::return_type
{
  // using executor_type =
  //   boost::asio::associated_executor_t<ConnectHandler,
  //                                      decltype(std::declval<::foxy::session&>().get_executor())>;

  boost::asio::async_completion<ConnectHandler,
                                void(boost::system::error_code, boost::asio::ip::tcp::endpoint)>
    init(handler);

  return ::foxy::detail::timer_wrap<::foxy::detail::connect_op>(*this, init, std::move(host),
                                                                std::move(service));
}

} // namespace foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
