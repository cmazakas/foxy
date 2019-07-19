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

namespace foxy
{
template <class Stream, class DynamicBuffer>
template <class Parser, class ReadHandler>
auto
basic_session<Stream, DynamicBuffer>::async_read_header(Parser& parser, ReadHandler&& handler) & ->
  typename boost::asio::async_result<std::decay_t<ReadHandler>,
                                     void(boost::system::error_code, std::size_t)>::return_type
{
  return boost::beast::http::async_read_header(stream, buffer, parser,
                                               std::forward<ReadHandler>(handler));
}

} // namespace foxy

#endif // FOXY_IMPL_SESSION_ASYNC_READ_HEADER_IMPL_HPP_
