//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SESSION_ASYNC_WRITE_HEADER_IMPL_HPP_
#define FOXY_IMPL_SESSION_ASYNC_WRITE_HEADER_IMPL_HPP_

#include <foxy/session.hpp>

namespace foxy
{
namespace detail
{

template <class Stream, class Serializer, class WriteHandler>
struct write_header_op : boost::asio::coroutine
{
private:

  struct state
  {
    ::foxy::basic_session<Stream>& session;
    Serializer&                    serializer;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      WriteHandler const&            handler,
      ::foxy::basic_session<Stream>& session_,
      Serializer&                    serializer_)
    : session(session_)
    , serializer(serializer_)
    , work(session.get_executor())
    {
    }
  };

  boost::beast::handler_ptr<state, WriteHandler> p_;

public:
  write_header_op()                       = delete;
  write_header_op(write_header_op const&) = default;
  write_header_op(write_header_op&&)      = default;

  template <class DeducedHandler>
  write_header_op(
    ::foxy::basic_session<Stream>& session,
    Serializer&                    serializer,
    DeducedHandler&&               handler)
  : p_(std::forward<DeducedHandler>(handler), session, serializer)
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    WriteHandler,
    decltype((std::declval<::foxy::basic_session<Stream>&>().get_executor()))
  >;

  using allocator_type = boost::asio::associated_allocator_t<WriteHandler>;

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

template <class Stream, class Serializer, class WriteHandler>
auto
write_header_op<Stream, Serializer, WriteHandler>::
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
    http::async_write_header(
      s.session.stream,
      s.serializer,
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
template <class Serializer, class WriteHandler>
auto
basic_session<Stream, X>::async_write_header(
  Serializer&    serializer,
  WriteHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t))
{
  boost::asio::async_completion<
    WriteHandler, void(boost::system::error_code, std::size_t)
  >
  init(handler);

  detail::timed_op_wrapper<
    Stream,
    detail::write_header_op,
    BOOST_ASIO_HANDLER_TYPE(
      WriteHandler,
      void(boost::system::error_code, std::size_t)),
    void(boost::system::error_code, std::size_t)
  >(*this, std::move(init.completion_handler)).template init<Stream, Serializer>(serializer);

  return init.result.get();
}

} // foxy

#endif // FOXY_IMPL_SESSION_ASYNC_WRITE_HEADER_IMPL_HPP_
