//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_SESSION_HPP
#define FOXY_SESSION_HPP

#include <foxy/multi_stream.hpp>
#include <foxy/type_traits.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/ssl/context.hpp>

#include <boost/mp11/bind.hpp>

#include <boost/beast/core/async_base.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/handler_ptr.hpp>

namespace foxy
{
struct session_opts
{
  using duration_type = typename boost::asio::steady_timer::duration;

  boost::optional<boost::asio::ssl::context&> ssl_ctx = {};
  duration_type                               timeout = std::chrono::seconds{1};
};

template <class Stream, class = std::enable_if_t<boost::beast::is_async_stream<Stream>::value>>
struct basic_session
{
public:
  using stream_type = ::foxy::basic_multi_stream<Stream>;
  using buffer_type = boost::beast::flat_buffer;
  using timer_type  = boost::asio::steady_timer;

  session_opts opts;
  stream_type  stream;
  buffer_type  buffer;
  timer_type   timer;

  basic_session()                     = delete;
  basic_session(basic_session const&) = delete;
  basic_session(basic_session&&)      = default;

  explicit basic_session(boost::asio::io_context& io, session_opts opts_ = {});
  explicit basic_session(stream_type stream_, session_opts opts_ = {});

  using executor_type = decltype(stream.get_executor());

  auto
  get_executor() -> executor_type;

  template <class Parser, class ReadHandler>
  auto
  async_read_header(Parser& parser, ReadHandler&& handler) & -> BOOST_ASIO_INITFN_RESULT_TYPE(
    ReadHandler,
    void(boost::system::error_code, std::size_t));

  template <class Parser, class ReadHandler>
  auto
  async_read(Parser& parser, ReadHandler&& handler) & -> BOOST_ASIO_INITFN_RESULT_TYPE(
    ReadHandler,
    void(boost::system::error_code, std::size_t));

  template <class Serializer, class WriteHandler>
  auto
  async_write_header(Serializer& serializer, WriteHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<WriteHandler>,
                                       void(boost::system::error_code, std::size_t)>::return_type;

  template <class Serializer, class WriteHandler>
  auto
  async_write(Serializer& serializer, WriteHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<WriteHandler>,
                                       void(boost::system::error_code, std::size_t)>::return_type;
};

using session = basic_session<
  boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::io_context::executor_type>>;

} // namespace foxy

#include <foxy/impl/session.impl.hpp>

#endif // FOXY_SESSION_HPP
