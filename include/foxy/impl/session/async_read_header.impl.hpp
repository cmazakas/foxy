//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SESSION_ASYNC_READ_HEADER_IMPL_HPP_
#define FOXY_IMPL_SESSION_ASYNC_READ_HEADER_IMPL_HPP_

#include <foxy/session.hpp>
#include <foxy/detail/timed_op_wrapper_v2.hpp>

namespace foxy
{
namespace detail
{
template <class Stream, class Parser, class Handler>
struct read_header_op
  : boost::beast::
      async_base<Handler, decltype(std::declval<::foxy::basic_session<Stream>&>().get_executor())>,
    boost::asio::coroutine
{
  ::foxy::basic_session<Stream>& session;
  Parser&                        parser;

  read_header_op()                      = default;
  read_header_op(read_header_op const&) = default;
  read_header_op(read_header_op&&)      = default;

  read_header_op(::foxy::basic_session<Stream>& session_, Handler handler, Parser& parser_)
    : boost::beast::async_base<Handler,
                               decltype(
                                 std::declval<::foxy::basic_session<Stream>&>().get_executor())>(
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
      boost::beast::http::async_read_header(session.stream, session.buffer, parser,
                                            std::move(*this));

      this->complete(is_continuation, ec, bytes_transferred);
    }
  }
};

} // namespace detail

template <class Stream>
template <class Parser, class ReadHandler>
auto
basic_session<Stream>::async_read_header(Parser& parser, ReadHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  boost::asio::async_completion<ReadHandler, void(boost::system::error_code, std::size_t)> init(
    handler);

  return ::foxy::detail::timer_wrap<
    boost::mp11::mp_bind_front<::foxy::detail::read_header_op, Stream, Parser>::template fn>(
    *this, init, parser);
}

} // namespace foxy

#endif // FOXY_IMPL_SESSION_ASYNC_READ_HEADER_IMPL_HPP_
