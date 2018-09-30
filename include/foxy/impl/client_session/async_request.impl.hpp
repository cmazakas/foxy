//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_

#include <foxy/client_session.hpp>

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
    ::foxy::session& session;
    Request&                request;
    ResponseParser&         parser;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      RequestHandler const&   handler,
      ::foxy::session& session_,
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
    ::foxy::session& session,
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
    decltype((std::declval<::foxy::session&>().get_executor()))
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

  detail::timed_op_wrapper<
    detail::request_op,
    BOOST_ASIO_HANDLER_TYPE(
      RequestHandler,
      void(boost::system::error_code)),
    void()
  >(*this, std::move(init.completion_handler)).template init<Request, ResponseParser>(request, parser);

  return init.result.get();
}

} // foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_REQUEST_IMPL_HPP_
