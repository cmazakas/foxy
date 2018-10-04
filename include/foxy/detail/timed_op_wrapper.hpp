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

template <
  class Stream,
  template <class, class...> class Op,
  class Handler,
  class Sig
>
struct timed_op_wrapper
{
private:
  // our state needs to store the result type of the main operation because
  // of the when_all delaying the execution of the final handler
  //
  struct state
  {
    using args_t = boost::callable_traits::args_t<Sig>;

    args_t                         args;
    ::foxy::basic_session<Stream>& session;
    int                            ops;
    boost::asio::coroutine         coro;
    bool                           done;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      Handler const&                 handler,
      ::foxy::basic_session<Stream>& session_)
    : session(session_)
    , ops{0}
    , coro()
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
  timed_op_wrapper(
    ::foxy::basic_session<Stream>& session,
    DeducedHandler&&               handler)
  : p_(std::forward<DeducedHandler>(handler), session)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    Handler,
    decltype(std::declval<::foxy::basic_session<Stream>&>().get_executor())
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
    (*this)(on_completion_t{}, {});

    p_.reset();
  }

  struct on_timer_t {};
  struct on_completion_t {};

  auto operator()(on_timer_t, boost::system::error_code ec) -> void
  {
    std::cout << "done timer op!\n";

    p_->ops++;
    if (ec == boost::asio::error::operation_aborted) {
      return (*this)(on_completion_t{}, boost::system::error_code());
    }

    if (
      p_->done ||
      p_->session.timer.expiry() > std::chrono::steady_clock::now()
    ) {
      return (*this)(on_completion_t{}, boost::system::error_code());
    }

    p_->session.stream.plain().close(ec);
    (*this)(on_completion_t{}, ec);
  }

  template <class ...Args>
  auto operator()(boost::system::error_code ec, Args&&... args) -> void
  {
    std::cout << "done with main op!\n";

    p_->ops++;
    p_->args = typename state::args_t(std::forward<Args>(args)...);
    p_->done = true;

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
        std::cout << "yielding for when_all (" << s.ops << ")\n";
        BOOST_ASIO_CORO_YIELD;
        std::cout << "done with an async op (" << s.ops << ")\n" ;
      }
    }

    if (!s.coro.is_complete()) { return; }

    std::cout << "when_all is complete\n";

    auto args = std::move(s.args);
    auto work = std::move(s.work);

    auto f = [&](auto&& ...args)
    {
      p_.invoke(ec, std::forward<decltype(args)>(args)...);
    };

    hof::unpack(f)(std::move(args));
  }
};

} // detail
} // foxy

#endif // FOXY_DETAIL_TIMED_OP_WRAPPER_HPP_
