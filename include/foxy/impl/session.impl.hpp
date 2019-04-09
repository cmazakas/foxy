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
template <class Stream>
foxy::basic_session<Stream>::basic_session(boost::asio::io_context& io, session_opts opts_)
  : opts(std::move(opts_))
  , stream(opts.ssl_ctx ? stream_type(io, *opts.ssl_ctx) : stream_type(io))
  , timer(io)
{
}

template <class Stream>
foxy::basic_session<Stream>::basic_session(stream_type stream_, session_opts opts_)
  : opts(std::move(opts_))
  , stream(std::move(stream_))
  , timer(stream.get_executor().context())
{
}

template <class Stream>
auto
foxy::basic_session<Stream>::get_executor() -> executor_type
{
  return stream.get_executor();
}

} // namespace foxy

#include <foxy/impl/session/async_read.impl.hpp>
#include <foxy/impl/session/async_write.impl.hpp>
#include <foxy/impl/session/async_read_header.impl.hpp>
#include <foxy/impl/session/async_write_header.impl.hpp>

#endif // FOXY_SESSION_IMPL_HPP_
