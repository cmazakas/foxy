//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_TIMED_OP_WRAPPER_V2_HPP_
#define FOXY_DETAIL_TIMED_OP_WRAPPER_V2_HPP_

#include <foxy/session.hpp>
#include <foxy/shared_handler_ptr.hpp>
#include <foxy/detail/close_stream.hpp>

#include <boost/beast/core/bind_handler.hpp>
#include <boost/system/error_code.hpp>
#include <boost/hof/unpack.hpp>

#include <tuple>

namespace foxy
{
namespace detail
{
template <class Stream, template <class...> class Op, class Handler, class Signature>
struct timed_op_wrapper_v2;

template <class Stream, template <class...> class Op, class Handler, class Ret, class... Args>
struct timed_op_wrapper_v2<Stream, Op, Handler, Ret(Args...)>
{
public:
  using executor_type =
    boost::asio::associated_executor_t<Handler,
                                       typename ::foxy::basic_session<Stream>::executor_type>;

  using allocator_type = boost::asio::associated_allocator_t<Handler>;

private:
  struct state
  {
    std::tuple<Args...>                             args;
    int                                             ops;
    boost::asio::coroutine                          coro;
    boost::asio::coroutine                          timer_coro;
    bool                                            done;
    boost::asio::executor_work_guard<executor_type> work;

    state(Handler const& handler, typename ::foxy::basic_session<Stream>::executor_type executor)
      : ops{0}
      , done{false}
      , work(boost::asio::get_associated_executor(handler, executor))
    {
    }
  };

  ::foxy::basic_session<Stream>&             session_;
  ::foxy::shared_handler_ptr<state, Handler> p_;

public:
  struct on_timer_t
  {
  };
  struct on_completion_t
  {
  };

  timed_op_wrapper_v2()                           = delete;
  timed_op_wrapper_v2(timed_op_wrapper_v2 const&) = default;
  timed_op_wrapper_v2(timed_op_wrapper_v2&&)      = default;

  template <class DeducedHandler, class... ConstructorArgs>
  timed_op_wrapper_v2(::foxy::basic_session<Stream>& session,
                      DeducedHandler&&               handler,
                      ConstructorArgs&&... args)
    : session_(session)
    , p_(std::forward<DeducedHandler>(handler), session_.get_executor())
  {
    // we will rely on the Op's constructor to be a non-blocking initiating function
    //
    Op<timed_op_wrapper_v2>(session, *this, std::forward<ConstructorArgs>(args)...);

    session_.timer.expires_after(session_.opts.timeout);
    session_.timer.async_wait(boost::beast::bind_front_handler(*this, on_timer_t{}));
    (*this)(on_completion_t{}, {});

    p_.reset();
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), session_.get_executor());
  }

  auto
  get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  // this is the hook that accepts the user's initiating function completing
  //
  auto
  operator()(Args... args) -> void
  {
    p_->done = true;
    p_->args = std::tuple<Args...>(std::move(args)...);
    p_->ops++;
    session_.timer.cancel();
    return (*this)(on_completion_t{}, {});
  }

  // the timer side of the wrapper
  //
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

        BOOST_ASIO_CORO_YIELD
        session_.timer.async_wait(boost::beast::bind_front_handler(*this, on_timer_t{}));
      }
    }

    if (!s.timer_coro.is_complete()) { return; }

    s.ops++;

    // we swallow the error here, may be a mistake later but for now, we're only interested in the
    // user's initiating function's error code
    //
    if (ec || s.done) { return (*this)(on_completion_t{}, {}); }

    auto& stream =
      session_.stream.is_ssl() ? session_.stream.ssl().next_layer() : session_.stream.plain();

    close(stream);
    (*this)(on_completion_t{}, {});
  }

  // this is our main flow controller and is responsible for invoking the user's final handler
  //
  auto
  operator()(on_completion_t, boost::system::error_code ec) -> void
  {
    auto& s = *p_;
    BOOST_ASIO_CORO_REENTER(s.coro)
    {
      while (s.ops < 2) { BOOST_ASIO_CORO_YIELD; }
    }

    if (!s.coro.is_complete()) { return; }

    auto args = std::move(s.args);
    auto work = std::move(s.work);

    auto f = [&](auto&&... args) { p_.invoke(std::forward<decltype(args)>(args)...); };
    boost::hof::unpack(f)(std::move(args));
  }
};

template <template <class...> class Op,
          class Stream,
          class CompletionToken,
          class Signature,
          class... Args>
auto
timer_wrap(::foxy::basic_session<Stream>&                             session,
           boost::asio::async_completion<CompletionToken, Signature>& init,
           Args&&... args)
{
  using handler_type =
    typename boost::asio::async_completion<CompletionToken, Signature>::completion_handler_type;

  timed_op_wrapper_v2<Stream, Op, handler_type, Signature>(
    session, std::move(init.completion_handler), args...);

  return init.result.get();
}
} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_TIMED_OP_WRAPPER_V2_HPP_
