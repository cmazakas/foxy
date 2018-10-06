//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#ifndef FOXY_MULTI_STREAM_HPP_
#define FOXY_MULTI_STREAM_HPP_

#include <boost/asio/io_context.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/type_traits.hpp>

#include <boost/asio/ssl/context.hpp>
#include <boost/beast/experimental/core/ssl_stream.hpp>

#include <boost/optional/optional.hpp>

#include <utility>
#include <type_traits>

namespace foxy
{

template <
  class Stream,
  class = std::enable_if_t<boost::beast::is_async_stream<Stream>::value>
>
struct basic_multi_stream
{
public:
  using stream_type     = Stream;
  using ssl_stream_type = boost::beast::ssl_stream<stream_type&>;
  using executor_type   = boost::asio::io_context::executor_type;

private:
  stream_type                      stream_;
  boost::optional<ssl_stream_type> ssl_stream_;

public:
  basic_multi_stream()                          = delete;
  basic_multi_stream(basic_multi_stream const&) = delete;
  basic_multi_stream(basic_multi_stream&&)      = default;

  explicit basic_multi_stream(boost::asio::io_context&);

  auto plain() & noexcept -> stream_type&;
  auto ssl()   & noexcept -> ssl_stream_type&;

  auto ssl(boost::asio::ssl::context ctx) -> void;

  auto is_ssl() const noexcept -> bool;

  auto get_executor() -> executor_type;

  // we inline these two method implementations so we don't have to declare
  // the return type
  //
  template <class MutableBufferSequence, class CompletionToken>
  auto
  async_read_some(MutableBufferSequence const& buffers, CompletionToken&& token)
  {
    if (ssl_stream_) {
      return ssl_stream_->async_read_some(buffers, std::forward<CompletionToken>(token));
    }
    return stream_.async_read_some(buffers, std::forward<CompletionToken>(token));
  }

  template <class ConstBufferSequence, class CompletionToken>
  auto
  async_write_some(ConstBufferSequence const& buffers, CompletionToken&& token)
  {
    if (ssl_stream_) {
      return ssl_stream_->async_write_some(buffers, std::forward<CompletionToken>(token));
    }
    return stream_.async_write_some(buffers, std::forward<CompletionToken>(token));
  }
};

// impl
//
template <class Stream, class X>
basic_multi_stream<Stream, X>::basic_multi_stream(boost::asio::io_context& io)
: stream_(io)
{
}

template <class Stream, class X>
auto
basic_multi_stream<Stream, X>::plain() & noexcept -> stream_type&
{
  return stream_;
}

template <class Stream, class X>
auto
basic_multi_stream<Stream, X>::ssl() & noexcept -> ssl_stream_type&
{
  return *ssl_stream_;
}

template <class Stream, class X>
auto
basic_multi_stream<Stream, X>::
ssl(boost::asio::ssl::context context) -> void
{

}

template <class Stream, class X>
auto
basic_multi_stream<Stream, X>::is_ssl() const noexcept -> bool
{
  return static_cast<bool>(ssl_stream_);
}

template <class Stream, class X>
auto
basic_multi_stream<Stream, X>::get_executor()
-> boost::asio::io_context::executor_type
{
  return stream_.get_executor();
}

extern template struct basic_multi_stream<boost::asio::ip::tcp::socket>;
using multi_stream = basic_multi_stream<boost::asio::ip::tcp::socket>;

} // foxy

#endif // FOXY_MULTI_STREAM_HPP_
