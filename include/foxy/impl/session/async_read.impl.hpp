//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_
#define FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_

#include <foxy/session.hpp>
#include <foxy/detail/timed_op_wrapper_v2.hpp>

namespace foxy
{
namespace detail
{
template <class Stream, class DynamicBuffer, class Parser, class Handler>
struct read_op
  : boost::beast::async_base<Handler,
                             typename ::foxy::basic_session<Stream, DynamicBuffer>::executor_type>,
    boost::asio::coroutine
{
  ::foxy::basic_session<Stream, DynamicBuffer>& session;
  Parser&                                       parser;

  read_op()               = default;
  read_op(read_op const&) = default;
  read_op(read_op&&)      = default;

  read_op(::foxy::basic_session<Stream, DynamicBuffer>& session_, Handler handler, Parser& parser_)
    : boost::beast::
        async_base<Handler, typename ::foxy::basic_session<Stream, DynamicBuffer>::executor_type>(
          std::move(handler),
          session_.get_executor())
    , session(session_)
    , parser(parser_)
  {
    (*this)({}, 0, false);
  }

  auto
  operator()(boost::system::error_code ec,
             std::size_t const         bytes_transferred,
             bool const                is_continuation = true) -> void
  {
    BOOST_ASIO_CORO_REENTER(*this)
    {
      BOOST_ASIO_CORO_YIELD
      boost::beast::http::async_read(session.stream, session.buffer, parser, std::move(*this));

      this->complete(is_continuation, ec, bytes_transferred);
    }
  }
};

} // namespace detail

template <class Stream, class DynamicBuffer>
template <class Parser, class ReadHandler>
auto
basic_session<Stream, DynamicBuffer>::async_read(Parser& parser, ReadHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  boost::asio::async_completion<ReadHandler, void(boost::system::error_code, std::size_t)> init(
    handler);

  return ::foxy::detail::timer_wrap<boost::mp11::mp_bind_front<::foxy::detail::read_op, Stream,
                                                               DynamicBuffer, Parser>::template fn>(
    *this, init, parser);
}

} // namespace foxy

#endif // FOXY_IMPL_SESSION_ASYNC_READ_IMPL_HPP_
