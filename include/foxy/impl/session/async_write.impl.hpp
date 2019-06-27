//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_IMPL_SESSION_ASYNC_WRITE_IMPL_HPP_
#define FOXY_IMPL_SESSION_ASYNC_WRITE_IMPL_HPP_

#include <foxy/session.hpp>
#include <foxy/detail/timed_op_wrapper_v2.hpp>

namespace foxy
{
namespace detail
{
template <class Stream, class Serializer, class Handler>
struct write_op
  : boost::beast::async_base<Handler, typename ::foxy::basic_session<Stream>::executor_type>,
    boost::asio::coroutine
{
  ::foxy::basic_session<Stream>& session;
  Serializer&                    serializer;

  write_op()                = default;
  write_op(write_op const&) = default;
  write_op(write_op&&)      = default;

  write_op(::foxy::basic_session<Stream>& session_, Handler handler, Serializer& serializer_)
    : boost::beast::async_base<Handler, typename ::foxy::basic_session<Stream>::executor_type>(
        std::move(handler),
        session_.get_executor())
    , session(session_)
    , serializer(serializer_)
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
      boost::beast::http::async_write(session.stream, serializer, std::move(*this));

      this->complete(is_continuation, ec, bytes_transferred);
    }
  }
};

} // namespace detail

template <class Stream, class DynamicBuffer>
template <class Serializer, class WriteHandler>
auto
basic_session<Stream, DynamicBuffer>::async_write(Serializer&    serializer,
                                                  WriteHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<WriteHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  boost::asio::async_completion<WriteHandler, void(boost::system::error_code, std::size_t)> init(
    handler);

  return ::foxy::detail::timer_wrap<
    boost::mp11::mp_bind_front<::foxy::detail::write_op, Stream, Serializer>::template fn>(
    *this, init, serializer);
}

} // namespace foxy

#endif // FOXY_IMPL_SESSION_ASYNC_WRITE_IMPL_HPP_
