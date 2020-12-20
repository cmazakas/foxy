//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_SESSION_HPP_
#define FOXY_SESSION_HPP_

#include <foxy/session_opts.hpp>
#include <foxy/multi_stream.hpp>
#include <foxy/type_traits.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/buffer.hpp>

#include <boost/asio/ssl/context.hpp>

#include <boost/mp11/bind.hpp>

#include <boost/beast/core/async_base.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace foxy
{
template <class Stream, class DynamicBuffer>
struct basic_session
{
public:
  static_assert(boost::beast::is_async_stream<Stream>::value,
                "Requirements on the Stream type were not met. Stream must be a Beast.AsyncStream");

  static_assert(boost::asio::is_dynamic_buffer<DynamicBuffer>::value,
                "Requirements on the DynamicBuffer type were not met. DynamicBuffer must be an "
                "Asio.DynamicBuffer");

  using stream_type   = ::foxy::basic_multi_stream<Stream>;
  using buffer_type   = DynamicBuffer;
  using timer_type    = boost::asio::steady_timer;
  using executor_type = typename stream_type::executor_type;

  session_opts opts;
  stream_type  stream;
  buffer_type  buffer;
  timer_type   timer;

  basic_session()                     = delete;
  basic_session(basic_session const&) = delete;
  basic_session(basic_session&&)      = default;

  template <class... BufferArgs>
  basic_session(boost::asio::any_io_executor executor, session_opts opts_, BufferArgs&&... bargs);

  template <class... BufferArgs>
  basic_session(boost::asio::io_context& io, session_opts opts_, BufferArgs&&... bargs);

  template <class... BufferArgs>
  basic_session(stream_type stream_, session_opts opts_, BufferArgs&&... bargs);

  auto
  get_executor() -> executor_type;

  template <class Parser, class ReadHandler>
  auto
  async_read_header(Parser& parser, ReadHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                       void(boost::system::error_code, std::size_t)>::return_type;

  template <class Parser, class ReadHandler>
  auto
  async_read(Parser& parser, ReadHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                       void(boost::system::error_code, std::size_t)>::return_type;

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

using session = basic_session<boost::asio::ip::tcp::socket, boost::beast::flat_buffer>;

} // namespace foxy

#include <foxy/impl/session.impl.hpp>

#endif // FOXY_SESSION_HPP_
