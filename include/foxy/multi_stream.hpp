//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_MULTI_STREAM_HPP_
#define FOXY_MULTI_STREAM_HPP_

#include <foxy/detail/variant2/variant.hpp>

#include <boost/asio/io_context.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/stream_traits.hpp>

#include <boost/asio/ssl/context.hpp>
#include <boost/beast/ssl/ssl_stream.hpp>

#include <utility>
#include <type_traits>

namespace foxy
{
template <class Stream, class = std::enable_if_t<boost::beast::is_async_stream<Stream>::value>>
struct basic_multi_stream
{
public:
  using stream_type     = Stream;
  using ssl_stream_type = boost::beast::ssl_stream<stream_type>;
  using executor_type   = boost::asio::io_context::executor_type;

private:
  boost::variant2::variant<stream_type, ssl_stream_type> stream_;

public:
  basic_multi_stream()                          = delete;
  basic_multi_stream(basic_multi_stream const&) = delete;
  basic_multi_stream(basic_multi_stream&&)      = default;

  template <class Arg>
  basic_multi_stream(Arg&& arg);

  template <class Arg>
  basic_multi_stream(Arg&& arg, boost::asio::ssl::context& ctx);

  auto
    plain() &
    noexcept -> stream_type&;
  auto
    ssl() &
    noexcept -> ssl_stream_type&;

  auto
  is_ssl() const noexcept -> bool;

  auto
  get_executor() -> executor_type;

  // we inline these two method implementations so we don't have to declare
  // the return type
  //
  template <class MutableBufferSequence, class CompletionToken>
  auto
  async_read_some(MutableBufferSequence const& buffers, CompletionToken&& token)
  {
    return boost::variant2::visit(
      [&](auto& stream) {
        return stream.async_read_some(buffers, std::forward<CompletionToken>(token));
      },
      stream_);
  }

  template <class ConstBufferSequence, class CompletionToken>
  auto
  async_write_some(ConstBufferSequence const& buffers, CompletionToken&& token)
  {
    return boost::variant2::visit(
      [&](auto& stream) {
        return stream.async_write_some(buffers, std::forward<CompletionToken>(token));
      },
      stream_);
  }
};

// impl
//

template <class Stream, class X>
template <class Arg>
basic_multi_stream<Stream, X>::basic_multi_stream(Arg&& arg)
  : stream_(stream_type(std::forward<Arg>(arg)))
{
}

template <class Stream, class X>
template <class Arg>
basic_multi_stream<Stream, X>::basic_multi_stream(Arg&& arg, boost::asio::ssl::context& ctx)
  : stream_(ssl_stream_type(std::forward<Arg>(arg), ctx))
{
}

template <class Stream, class X>
  auto
  basic_multi_stream<Stream, X>::plain() &
  noexcept -> stream_type&
{
  return boost::variant2::get<stream_type>(stream_);
}

template <class Stream, class X>
  auto
  basic_multi_stream<Stream, X>::ssl() &
  noexcept -> ssl_stream_type&
{
  return boost::variant2::get<ssl_stream_type>(stream_);
}

template <class Stream, class X>
auto
basic_multi_stream<Stream, X>::is_ssl() const noexcept -> bool
{
  return stream_.index() == 1;
}

template <class Stream, class X>
auto
basic_multi_stream<Stream, X>::get_executor() -> executor_type
{
  return boost::variant2::visit([](auto& stream) { return stream.get_executor(); }, stream_);
}

using multi_stream = basic_multi_stream<
  boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::io_context::executor_type>>;

} // namespace foxy

#endif // FOXY_MULTI_STREAM_HPP_
