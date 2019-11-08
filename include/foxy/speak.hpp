//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/client_session.hpp>
#include <foxy/log.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/compose.hpp>

#include <string>
#include <memory>

namespace foxy
{
template <class RequestFactory>
auto
speak(boost::asio::executor ex,
      std::string           host_,
      std::string           service_,
      RequestFactory&&      request_factory_)
{
  struct speak_op : boost::asio::coroutine
  {
    using executor_type = boost::asio::executor;

    struct frame
    {
      ::foxy::client_session client_session;
      std::string            host;
      std::string            service;
      RequestFactory         request_factory;
    };

    boost::asio::executor  executor;
    std::unique_ptr<frame> p_;

    speak_op()                = delete;
    speak_op(speak_op const&) = delete;
    speak_op(speak_op&&)      = default;

    speak_op(boost::asio::executor executor_,
             std::string           host__,
             std::string           service__,
             RequestFactory&&      factory)
      : executor(executor_)
      , p_(new frame{
          ::foxy::client_session{executor, ::foxy::session_opts{{}, std::chrono::seconds(30)}},
          std::move(host__), std::move(service__), std::move(factory)})
    {
    }

    auto
    get_executor() const noexcept -> executor_type
    {
      return executor;
    }

    auto operator()(boost::system::error_code ec = {}, std::size_t bytes_transferred = 0)
    {
      auto& s = *p_;
      BOOST_ASIO_CORO_REENTER(*this)
      {
        BOOST_ASIO_CORO_YIELD s.client_session.async_connect(
          std::move(s.host), std::move(s.service), std::move(*this));

        if (ec) {
          ::foxy::log_error(ec, "speak::async_connect");
          return;
        }

        BOOST_ASIO_CORO_YIELD
        boost::asio::async_compose<speak_op, void(boost::system::error_code, std::size_t)>(
          s.request_factory(s.client_session), *this, executor);

        BOOST_ASIO_CORO_YIELD
        s.client_session.async_shutdown(std::move(*this));

        if (ec) { ::foxy::log_error(ec, "speak::async_shutdown"); }
      }
    }
  };

  boost::asio::post(speak_op(ex, std::move(host_), std::move(service_),
                             std::forward<RequestFactory>(request_factory_)));
}
} // namespace foxy
