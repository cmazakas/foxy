#ifndef FOXY_DETAIL_TIMED_OP_WRAPPER_HPP_
#define FOXY_DETAIL_TIMED_OP_WRAPPER_HPP_

#include <foxy/session.hpp>
#include <foxy/shared_handler_ptr.hpp>

#include <boost/callable_traits/args.hpp>
#include <boost/hof/unpack.hpp>

namespace foxy
{
namespace detail
{

template <template <class, class...> class Op, class Handler, class Sig>
struct timed_op_wrapper
{
private:
  // our state needs to store the result type of the main operation because
  // of the when_all delaying the execution of the final handler
  //
  struct state
  {
    using args_t = boost::callable_traits::args_t<Sig>;

    args_t                 args;
    ::foxy::session&       session;
    int                    ops;
    boost::asio::coroutine coro;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      Handler const&   handler_,
      ::foxy::session& session_)
    : session(session_)
    , ops{0}
    , coro()
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
  timed_op_wrapper(
    ::foxy::session& session,
    DeducedHandler&& handler)
  : p_(std::forward<DeducedHandler>(handler), session)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    Handler,
    decltype(std::declval<::foxy::session&>().get_executor())
  >;

  auto get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
  }

  using allocator_type = boost::asio::associated_allocator_t<Handler>;

  auto get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  template <class ...Types, class ...Args>
  auto
  init(Args&&... args) -> void
  {
    using boost::beast::bind_handler;
    using namespace std::placeholders;

    auto& s = *p_;
    Op<Types..., typename std::decay<decltype(*this)>::type>(
        s.session, std::forward<Args>(args)..., *this
      )({}, 0, false);

    s.session.timer.expires_after(s.session.opts.timeout);
    s.session.timer.async_wait(bind_handler(*this, on_timer_t{}, _1));

    p_.reset();
  }

  struct on_timer_t {};
  struct on_completion_t {};

  auto operator()(on_timer_t, boost::system::error_code ec) -> void
  {
    p_->ops++;
    if (ec == boost::asio::error::operation_aborted) {
      return (*this)(on_completion_t{}, boost::system::error_code());
    }

    p_->session.stream.plain().close(ec);
    (*this)(on_completion_t{}, ec);
  }

  template <class ...Args>
  auto operator()(boost::system::error_code ec, Args&&... args) -> void
  {
    p_->ops++;
    p_->args = typename state::args_t(std::forward<Args>(args)...);

    auto ec2 = ec;
    p_->session.timer.cancel(ec2);

    return (*this)(on_completion_t{}, ec);
  }

  auto operator()(on_completion_t, boost::system::error_code ec) -> void
  {
    namespace hof = boost::hof;

    auto& s = *p_;
    BOOST_ASIO_CORO_REENTER(s.coro)
    {
      while (s.ops < 2) {
        BOOST_ASIO_CORO_YIELD;
      }
    }

    if (!s.coro.is_complete()) { return; }

    auto args = std::move(s.args);
    auto work = std::move(s.work);

    auto f = [&](auto&& ...args)
    {
      p_.invoke(ec, std::forward<decltype(args)>(args)...);
    };

    hof::unpack(f)(std::move(args));
  }
};

// template <template <class ...Ts> class Op, class Handler, class Sig>
// auto
// timed_op_wrapper<Op<Op::Ts...>, Handler, Sig>::
// operator()(
//   on_timer_t,
//   boost::system::error_code ec) -> void
// {
//   p_->ops++;
//   if (ec == boost::asio::error::operation_aborted) {
//     return (*this)(on_completion_t{}, boost::system::error_code());
//   }

//   p_->session.stream.plain().close(ec);
//   (*this)(on_completion_t{}, ec);
// }

// template <template <class...> class Op, class Handler, class Sig>
// template <class ...Args>
// auto
// timed_op_wrapper<Op, Handler, Sig>::
// operator()(boost::system::error_code ec, Args&&... args) -> void
// {
//   p_->ops++;
//   p_->args = state::args_t(std::forward<Args>(args)...);

//   auto ec2 = ec;
//   p_->session.timer.cancel(ec2);

//   return (*this)(on_completion_t{}, ec);
// }

// template <template <class...> class Op, class Handler, class Sig>
// auto
// timed_op_wrapper<Op, Handler, Sig>::
// operator()(on_completion_t, boost::system::error_code ec) -> void
// {
//   namespace hof = boost::hof;

//   auto& s = *p_;
//   BOOST_ASIO_CORO_REENTER(s.coro)
//   {
//     while (s.ops < 2) {
//       BOOST_ASIO_CORO_YIELD;
//     }
//   }

//   if (!s.coro.is_complete()) { return; }

//   auto args = std::move(s.args);
//   auto work = std::move(s.work);

//   auto f = [&](auto&& ...args)
//   {
//     p_.invoke(ec, std::forward<decltype(args)>(args)...);
//   };

//   hof::unpack(f)(std::move(args));
// }

} // detail
} // foxy

#endif // FOXY_DETAIL_TIMED_OP_WRAPPER_HPP_
