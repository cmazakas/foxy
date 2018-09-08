#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_

#include "foxy/client_session.hpp"

namespace foxy
{
namespace detail
{

template <class Request, class ResponseParser, class RequestHandler>
struct request_op : boost::asio::coroutine
{
private:

  struct state
  {
    ::foxy::client_session& session;
    Request&                request;
    ResponseParser&         parser;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      RequestHandler const&   handler,
      ::foxy::client_session& session_,
      Request&                request_,
      ResponseParser&         parser_)
    : session(session_)
    , request(request_)
    , parser(parser_)
    , work(session.get_executor())
    {
    }
  };

  boost::beast::handler_ptr<state, RequestHandler> p_;

public:
  request_op()                  = delete;
  request_op(request_op const&) = default;
  request_op(request_op&&)      = default;

  template <class DeducedHandler>
  request_op(
    ::foxy::client_session& session,
    Request&                request,
    ResponseParser&         parser,
    DeducedHandler&&        handler)
  : p_(
    std::forward<DeducedHandler>(handler),
    session, request, parser)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    RequestHandler,
    decltype((std::declval<::foxy::client_session&>().get_executor()))
  >;

  using allocator_type = boost::asio::associated_allocator_t<RequestHandler>;

  auto get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
  }

  auto get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  auto
  operator()(
    boost::system::error_code ec,
    std::size_t const         bytes_transferred,
    bool const                is_continuation = true) -> void;
};

template <class Request, class ResponseParser, class RequestHandler>
auto
request_op<Request, ResponseParser, RequestHandler>::operator()(
  boost::system::error_code ec,
  std::size_t const         bytes_transferred,
  bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  namespace http = boost::beast::http;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    BOOST_ASIO_CORO_YIELD
    http::async_write(s.session.stream, s.request, std::move(*this));
    if (ec) { goto upcall; }

    BOOST_ASIO_CORO_YIELD
    http::async_read(s.session.stream, s.session.buffer, s.parser, std::move(*this));
    if (ec) { goto upcall; }

    {
      auto work = std::move(s.work);
      return p_.invoke(boost::system::error_code());
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec);
  }
}

template <class Request, class ResponseParser, class RequestHandler>
struct request_op_main
{
private:

  struct state
  {
    ::foxy::client_session&        session;
    Request&                       request;
    ResponseParser&                parser;

    int                            ops_completed;
    boost::asio::coroutine         coro;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      RequestHandler const&   handler_,
      ::foxy::client_session& session_,
      Request&                request_,
      ResponseParser&         parser_)
    : session(session_)
    , request(request_)
    , parser(parser_)
    , ops_completed{0}
    , coro()
    , work(session.get_executor())
    {
    }
  };

  ::foxy::shared_handler_ptr<state, RequestHandler> p_;

public:
  request_op_main()                       = delete;
  request_op_main(request_op_main const&) = default;
  request_op_main(request_op_main&&)      = default;

  template <class DeducedHandler>
  request_op_main(
    ::foxy::client_session& session,
    Request&                request,
    ResponseParser&         parser,
    DeducedHandler&&        handler)
  : p_(std::forward<DeducedHandler>(handler), session, request, parser)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    RequestHandler,
    decltype(std::declval<::foxy::client_session&>().get_executor())
  >;

  auto get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
  }

  using allocator_type = boost::asio::associated_allocator_t<RequestHandler>;

  auto get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  struct on_timer_t {};
  struct on_request_t {};

  auto operator()(on_timer_t, boost::system::error_code ec) -> void;
  auto operator()(boost::system::error_code ec, bool const is_continuation = true) -> void;
  auto operator()(on_request_t, boost::system::error_code ec) -> void;
};

template <class Request, class ResponseParser, class RequestHandler>
auto
request_op_main<Request, ResponseParser, RequestHandler>::
operator()(
  on_timer_t,
  boost::system::error_code ec) -> void
{
  p_->ops_completed++;
  if (ec == boost::asio::error::operation_aborted) {
    return (*this)(boost::system::error_code());
  }
  (*this)(ec);
}

template <class Request, class ResponseParser, class RequestHandler>
auto
request_op_main<Request, ResponseParser, RequestHandler>::
operator()(
  on_request_t,
  boost::system::error_code ec) -> void
{
  p_->ops_completed++;
  if (ec == boost::asio::error::operation_aborted) {
    return (*this)(boost::system::error_code());
  }
  (*this)(ec);
}

template <class Request, class ResponseParser, class RequestHandler>
auto
request_op_main<Request, ResponseParser, RequestHandler>::
operator()(
  boost::system::error_code ec,
  bool const                is_continuation) -> void
{
  using namespace std::chrono_literals;
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  auto& s = *p_;

  BOOST_ASIO_CORO_REENTER(s.coro)
  {
    s.session.timer.expires_after(1s);

    {
      auto h = bind_handler(*this, on_request_t{}, _1);

      request_op<
        Request, ResponseParser,
        decltype(h)
      >(s.session, s.request, s.parser, std::move(h))({}, false);
    }

    s.session.timer.async_wait(bind_handler(*this, on_timer_t{}, _1));

    p_.reset();
    while (s.opts_completed < 2) {
      BOOST_ASIO_CORO_YIELD;
    }

    if (ec) { goto upcall; }

    return;

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(bind_handler(*this, ec));
    }
  }

  if (!s.coro.is_complete()) { return; }

  auto work     = std::move(s.work);

  if (ec) { return p_.invoke(ec); };
  return p_.invoke(ec);
}

} // detail

template <class Request, class ResponseParser, class RequestHandler>
auto
client_session::async_request(
  Request&         request,
  ResponseParser&  parser,
  RequestHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(RequestHandler, void(boost::system::error_code))
{
  boost::asio::async_completion<
    RequestHandler, void(boost::system::error_code)
  >
  init(handler);

  detail::request_op<
    Request, ResponseParser,
    BOOST_ASIO_HANDLER_TYPE(RequestHandler, void(boost::system::error_code))
  >(*this, request, parser, std::move(init.completion_handler))({}, 0, false);

  return init.result.get();
}

} // foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_
