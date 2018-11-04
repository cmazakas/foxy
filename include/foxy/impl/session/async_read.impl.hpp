//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_
#define FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_

#include <foxy/session.hpp>

namespace foxy
{
namespace detail
{

template <class Stream, class Parser, class ReadHandler>
struct read_op : boost::asio::coroutine
{
private:

  struct state
  {
    ::foxy::basic_session<Stream>& session;
    Parser&                        parser;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      ReadHandler const&             handler,
      ::foxy::basic_session<Stream>& session_,
      Parser&                        parser_)
    : session(session_)
    , parser(parser_)
    , work(session.get_executor())
    {
    }
  };

  boost::beast::handler_ptr<state, ReadHandler> p_;

public:
  read_op()                      = delete;
  read_op(read_op const&) = default;
  read_op(read_op&&)      = default;

  template <class DeducedHandler>
  read_op(
    ::foxy::basic_session<Stream>& session,
    Parser&                        parser,
    DeducedHandler&&               handler)
  : p_(std::forward<DeducedHandler>(handler), session, parser)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    ReadHandler,
    decltype((std::declval<::foxy::basic_session<Stream>&>().get_executor()))
  >;

  using allocator_type = boost::asio::associated_allocator_t<ReadHandler>;

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

template <class Stream, class Parser, class ReadHandler>
auto
read_op<Stream, Parser, ReadHandler>::
operator()(
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
    http::async_read(
      s.session.stream,
      s.session.buffer,
      s.parser,
      std::move(*this));

    if (ec) { goto upcall; }

    {
      auto work = std::move(s.work);
      return p_.invoke(boost::system::error_code(), bytes_transferred);
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec, 0);
  }
}

} // detail

template <class Stream, class X>
template <class Parser, class ReadHandler>
auto
basic_session<Stream, X>::async_read(
  Parser&       parser,
  ReadHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t))
{
  boost::asio::async_completion<
    ReadHandler, void(boost::system::error_code, std::size_t)
  >
  init(handler);

  detail::timed_op_wrapper<
    Stream,
    detail::read_op,
    BOOST_ASIO_HANDLER_TYPE(
      ReadHandler,
      void(boost::system::error_code, std::size_t)),
    void(boost::system::error_code, std::size_t)
  >(*this, std::move(init.completion_handler)).template init<Stream, Parser>(parser);

  return init.result.get();
}

} // foxy

#endif // FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_
