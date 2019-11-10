//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_TIMED_OP_WRAPPER_V3_HPP_
#define FOXY_DETAIL_TIMED_OP_WRAPPER_V3_HPP_

#include <foxy/session.hpp>
#include <foxy/detail/close_stream.hpp>

#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/associated_allocator.hpp>

#include <boost/beast/core/bind_handler.hpp>

#include <boost/optional/optional.hpp>
#include <boost/hof/unpack.hpp>

#include <boost/system/error_code.hpp>

#include <boost/assert.hpp>

#include <memory>
#include <utility>
#include <tuple>
#include <type_traits>

namespace foxy
{
namespace detail
{
template <class Signature>
struct async_timer_initiation;

template <class Ret, class... Args>
struct async_timer_initiation<Ret(Args...)>
{
  template <class CompletionHandler, class Implementation, class Stream, class DynamicBuffer>
  auto
  operator()(CompletionHandler&&                           handler,
             Implementation&&                              implementation,
             ::foxy::basic_session<Stream, DynamicBuffer>& session) const -> void
  {
    struct on_timer_t
    {
    };

    struct state
    {
      std::decay_t<CompletionHandler>      handler_;
      boost::optional<std::tuple<Args...>> results = {};
      boost::asio::coroutine               timer_coro;
      bool                                 done = false;

      state()             = delete;
      state(state const&) = delete;
      state(state&&)      = default;

      state(CompletionHandler&& deduced_handler)
        : handler_(std::move(deduced_handler))
      {
      }
    };

    struct intermediate_completion_handler
    {
      using executor_type = boost::asio::associated_executor_t<
        std::decay_t<CompletionHandler>,
        typename ::foxy::basic_session<Stream, DynamicBuffer>::executor_type>;

      using allocator_type = boost::asio::associated_allocator_t<std::decay_t<CompletionHandler>>;

      std::shared_ptr<state>                        p_;
      ::foxy::basic_session<Stream, DynamicBuffer>& session_;

      intermediate_completion_handler()                                       = delete;
      intermediate_completion_handler(intermediate_completion_handler const&) = default;
      intermediate_completion_handler(intermediate_completion_handler&&)      = default;

      intermediate_completion_handler(CompletionHandler&& completion_handler,
                                      ::foxy::basic_session<Stream, DynamicBuffer>& session)
        : p_(std::allocate_shared<state>(boost::asio::get_associated_allocator(completion_handler),
                                         std::move(completion_handler)))
        , session_(session)
      {
      }

      auto
      get_executor() const noexcept -> executor_type
      {
        return boost::asio::get_associated_executor(p_->handler_, session_.get_executor());
      }

      auto
      get_allocator() const noexcept -> allocator_type
      {
        return boost::asio::get_associated_allocator(p_->handler_);
      }

      auto
      operator()(Args&&... args) -> void
      {
        p_->results = std::make_tuple(std::move(args)...);

        // if we're already marked as done here, it means the time naturally expired so our
        // user's async op was cancelled
        // deallocate the state and invoke the user's completion handler
        //
        if (p_->done) {
          auto results = std::move(*(p_->results));
          auto cb      = std::move(p_->handler_);

          p_.reset();
          BOOST_ASSERT(p_.use_count() == 0);

          auto f = [&](auto&&... args) { cb(std::forward<decltype(args)>(args)...); };
          boost::hof::unpack(f)(std::move(results));
          return;
        }

        // our user's async op completed before the timer did, mark the op as true and cancel
        // the pending cancel op
        //
        p_->done = true;
        session_.timer.cancel();
      }

      auto
      operator()(on_timer_t, boost::system::error_code ec) -> void
      {
        auto& s = *p_;
        BOOST_ASIO_CORO_REENTER(s.timer_coro)
        {
          // we only get this error code if `expires_after` is called or our timer was `cancel`d
          //
          while (ec == boost::asio::error::operation_aborted) {
            // to differentiate between an elongation of the timer operation and the user's wrapped
            // initiating function completing, we use a simple `done` flag
            //
            if (s.done) { break; }

            session_.timer.expires_after(session_.opts.timeout);

            BOOST_ASIO_CORO_YIELD
            session_.timer.async_wait(
              boost::beast::bind_front_handler(std::move(*this), on_timer_t{}));
          }
        }

        if (!s.timer_coro.is_complete()) { return; }

        if (s.done) {
          auto args = std::move(*(s.results));
          auto cb   = std::move(s.handler_);

          p_.reset();
          BOOST_ASSERT(p_.use_count() == 0);

          auto f = [&](auto&&... args) { cb(std::forward<decltype(args)>(args)...); };
          boost::hof::unpack(f)(std::move(args));
          return;
        }

        // maybe handle if (ec) { ... } here
        //

        // the timer expired naturally, mark the op as done and close the stream
        //
        p_->done = true;

        auto& stream =
          session_.stream.is_ssl() ? session_.stream.ssl().next_layer() : session_.stream.plain();

        close(stream);
      }
    };

    auto intermediate_handler =
      intermediate_completion_handler(std::forward<CompletionHandler>(handler), session);

    session.timer.expires_after(session.opts.timeout);
    session.timer.async_wait(boost::beast::bind_front_handler(
      static_cast<intermediate_completion_handler const&>(intermediate_handler), on_timer_t{}));

    boost::asio::async_compose<intermediate_completion_handler, Ret(Args...)>(
      std::move(implementation), intermediate_handler, session.stream.get_executor(),
      session.timer.get_executor());
  }
};

template <class Signature,
          class CompletionToken,
          class Implementation,
          class Stream,
          class DynamicBuffer>
auto
async_timer(Implementation&&                              implementation,
            ::foxy::basic_session<Stream, DynamicBuffer>& session,
            CompletionToken&&                             token) ->
  typename boost::asio::async_result<std::decay_t<CompletionToken>, Signature>::return_type
{
  return boost::asio::async_initiate<CompletionToken, Signature>(
    async_timer_initiation<Signature>{}, token, std::move(implementation), session);
}
} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_TIMED_OP_WRAPPER_V3_HPP_
