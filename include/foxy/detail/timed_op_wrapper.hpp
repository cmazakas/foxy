//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_TIMED_OP_WRAPPER_HPP_
#define FOXY_DETAIL_TIMED_OP_WRAPPER_HPP_

#include <foxy/session.hpp>
#include <foxy/shared_handler_ptr.hpp>
#include <foxy/detail/close_stream.hpp>

#include <boost/callable_traits/args.hpp>
#include <boost/hof/unpack.hpp>

namespace foxy
{
namespace detail
{
template <class Stream, template <class, class...> class Op, class Handler, class Sig>
struct timed_op_wrapper
{
private:
  // our state needs to store the result type of the main operation because of the when_all delaying
  // the execution of the final handler
  //
  struct state
  {
    using args_t = boost::callable_traits::args_t<Sig>;

    args_t                         args;
    ::foxy::basic_session<Stream>& session;
    int                            ops;
    boost::asio::coroutine         coro;
    boost::asio::coroutine         timer_coro;
    bool                           done;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(Handler const& handler, ::foxy::basic_session<Stream>& session_)
      : session(session_)
      , ops{0}
      , done{false}
      , work(session.get_executor())
    {
    }
  };

  ::foxy::shared_handler_ptr<state, Handler> p_;

public:
  timed_op_wrapper()                        = delete;
  timed_op_wrapper(timed_op_wrapper const&) = default;
  timed_op_wrapper(timed_op_wrapper&&)      = default;

  template <class DeducedHandler>
  timed_op_wrapper(::foxy::basic_session<Stream>& session, DeducedHandler&& handler)
    : p_(std::forward<DeducedHandler>(handler), session)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    Handler,
    decltype(std::declval<::foxy::basic_session<Stream>&>().get_executor())>;

  auto
  get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
  }

  using allocator_type = boost::asio::associated_allocator_t<Handler>;

  auto
  get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  template <class... Types, class... Args>
  auto
  init(Args&&... args) -> void
  {
    using boost::beast::bind_handler;
    using namespace std::placeholders;

    auto& s = *p_;
    Op<Types..., typename std::decay<decltype(*this)>::type>(s.session, std::forward<Args>(args)...,
                                                             *this)({}, 0, false);

    s.session.timer.expires_after(s.session.opts.timeout);
    s.session.timer.async_wait(bind_front_handler(*this, on_timer_t{}));
    (*this)(on_completion_t{}, {});

    p_.reset();
  }

  struct on_timer_t
  {
  };
  struct on_completion_t
  {
  };

  auto
  operator()(on_timer_t, boost::system::error_code ec) -> void
  {
    using namespace std::placeholders;

    auto& s = *p_;
    BOOST_ASIO_CORO_REENTER(s.timer_coro)
    {
      // this means the user called `expires_after` _or_ our timer was cancelled
      //
      while (ec == boost::asio::error::operation_aborted) {
        // we know that we were cancelled if `p_->done` is true
        // note that this implies a precondition that no one  else will be calling cancel on the
        // timer
        //
        if (s.done) { break; }

        // otherwise, the user likely updated the timer's expiration so we need to prolong the async
        // operation accordingly
        //
        BOOST_ASIO_CORO_YIELD
        s.session.timer.async_wait(boost::beast::bind_front_handler(*this, on_timer_t{}));
      }
    }

    if (!s.timer_coro.is_complete()) { return; }

    s.ops++;
    if (ec || s.done) { return (*this)(on_completion_t{}, {}); }

    auto& stream =
      s.session.stream.is_ssl() ? s.session.stream.ssl().next_layer() : s.session.stream.plain();

    close(stream);
    (*this)(on_completion_t{}, {});
  }

  template <class... Args>
  auto
  operator()(Args&&... args) -> void
  {
    p_->ops++;

    p_->args = typename state::args_t(std::forward<Args>(args)...);
    p_->done = true;

    p_->session.timer.cancel();

    return (*this)(on_completion_t{}, {});
  }

  auto
  operator()(on_completion_t, boost::system::error_code ec) -> void
  {
    namespace hof = boost::hof;

    auto& s = *p_;
    BOOST_ASIO_CORO_REENTER(s.coro)
    {
      while (s.ops < 2) { BOOST_ASIO_CORO_YIELD; }
    }

    if (!s.coro.is_complete()) { return; }

    auto args = std::move(s.args);
    auto work = std::move(s.work);

    auto f = [&](auto&&... args) { p_.invoke(std::forward<decltype(args)>(args)...); };

    hof::unpack(f)(std::move(args));
  }
};

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_TIMED_OP_WRAPPER_HPP_
