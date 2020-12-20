//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_SESSION_IMPL_HPP_
#define FOXY_SESSION_IMPL_HPP_

#include <foxy/session.hpp>

namespace foxy
{
template <class Stream, class DynamicBuffer>
template <class... BufferArgs>
foxy::basic_session<Stream, DynamicBuffer>::basic_session(boost::asio::any_io_executor executor,
                                                          session_opts          opts_,
                                                          BufferArgs&&... bargs)
  : opts(std::move(opts_))
  , stream(opts.ssl_ctx ? stream_type(executor, *opts.ssl_ctx) : stream_type(executor))
  , buffer(std::forward<BufferArgs>(bargs)...)
  , timer(executor)
{
}

template <class Stream, class DynamicBuffer>
template <class... BufferArgs>
foxy::basic_session<Stream, DynamicBuffer>::basic_session(boost::asio::io_context& io,
                                                          session_opts             opts_,
                                                          BufferArgs&&... bargs)
  : opts(std::move(opts_))
  , stream(opts.ssl_ctx ? stream_type(io, *opts.ssl_ctx) : stream_type(io))
  , buffer(std::forward<BufferArgs>(bargs)...)
  , timer(io)
{
}

template <class Stream, class DynamicBuffer>
template <class... BufferArgs>
foxy::basic_session<Stream, DynamicBuffer>::basic_session(stream_type  stream_,
                                                          session_opts opts_,
                                                          BufferArgs&&... bargs)
  : opts(std::move(opts_))
  , stream(std::move(stream_))
  , buffer(std::forward<BufferArgs>(bargs)...)
  , timer(stream.get_executor())
{
}

template <class Stream, class DynamicBuffer>
auto
foxy::basic_session<Stream, DynamicBuffer>::get_executor() -> executor_type
{
  return stream.get_executor();
}

} // namespace foxy

#include <foxy/impl/session/async_read.impl.hpp>
#include <foxy/impl/session/async_write.impl.hpp>
#include <foxy/impl/session/async_read_header.impl.hpp>
#include <foxy/impl/session/async_write_header.impl.hpp>

#endif // FOXY_SESSION_IMPL_HPP_
