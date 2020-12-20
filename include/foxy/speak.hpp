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

#include <boost/smart_ptr/allocate_unique.hpp>

#include <functional>
#include <string>
#include <memory>

namespace foxy
{
template <class RequestFactory, class Allocator = std::allocator<char>>
auto
speak(boost::asio::any_io_executor ex,
      std::string           host_,
      std::string           service_,
      RequestFactory&&      request_factory_,
      session_opts          opts_ = {},
      Allocator const       alloc = {})
{
  struct speak_op : boost::asio::coroutine
  {
    using executor_type  = boost::asio::any_io_executor;
    using allocator_type = Allocator;

    struct frame
    {
      ::foxy::client_session client_session;
      std::string            host;
      std::string            service;
      RequestFactory         request_factory;
    };

    executor_type  executor;
    allocator_type allocator;

    std::unique_ptr<frame, boost::alloc_deleter<frame, allocator_type>> p_;

    speak_op()                = delete;
    speak_op(speak_op const&) = delete;
    speak_op(speak_op&&)      = default;

    speak_op(boost::asio::any_io_executor executor_,
             std::string           host__,
             std::string           service__,
             RequestFactory&&      factory,
             session_opts          opts__,
             allocator_type        alloc_)
      : executor(executor_)
      , allocator(alloc_)
      , p_(boost::allocate_unique<frame>(allocator,
                                         {::foxy::client_session(executor, opts__),
                                          std::move(host__), std::move(service__),
                                          std::move(factory)}))
    {
    }

    auto
    get_executor() const noexcept -> executor_type
    {
      return executor;
    }

    auto
    get_allocator() const noexcept -> allocator_type
    {
      return allocator;
    }

    auto operator()(boost::system::error_code ec = {}, std::size_t bytes_transferred = 0)
    {
      auto& s = *p_;
      BOOST_ASIO_CORO_REENTER(*this)
      {
        BOOST_ASIO_CORO_YIELD s.client_session.async_connect(
          std::move(s.host), std::move(s.service), std::move(*this));

        BOOST_ASIO_CORO_YIELD
        boost::asio::async_compose<speak_op, void(boost::system::error_code, std::size_t)>(
          std::bind(s.request_factory(s.client_session), std::placeholders::_1,
                    ec ? ec : boost::system::error_code{}, 0),
          *this, executor);

        BOOST_ASIO_CORO_YIELD s.client_session.async_shutdown(std::move(*this));
      }
    }
  };

  boost::asio::post(speak_op(ex, std::move(host_), std::move(service_),
                             std::forward<RequestFactory>(request_factory_), opts_, alloc));
}
} // namespace foxy
