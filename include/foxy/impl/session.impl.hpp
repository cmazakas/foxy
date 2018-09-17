//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#ifndef FOXY_SESSION_IMPL_HPP_
#define FOXY_SESSION_IMPL_HPP_

#include "foxy/session.hpp"

namespace foxy
{

template <class Stream, class X>
template <class Parser, class ReadHandler>
auto
basic_session<Stream, X>::async_read_header(
  Parser&       parser,
  ReadHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t))
{
  return boost::beast::http::async_read_header(
    stream,
    buffer,
    parser,
    std::forward<ReadHandler>(handler));
}

template <class Stream, class X>
template <class Parser, class ReadHandler>
auto
basic_session<Stream, X>::async_read(
  Parser&       parser,
  ReadHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(ReadHandler, void(boost::system::error_code, std::size_t))
{
  return boost::beast::http::async_read(
    stream,
    buffer,
    parser,
    std::forward<ReadHandler>(handler));
}

template <class Stream, class X>
template <class Serializer, class WriteHandler>
auto
basic_session<Stream, X>::async_write_header(
  Serializer&    serializer,
  WriteHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t))
{
  return boost::beast::http::async_write_header(
    stream,
    serializer,
    std::forward<WriteHandler>(handler));
}

template <class Stream, class X>
template <class Serializer, class WriteHandler>
auto
basic_session<Stream, X>::async_write(
  Serializer&    serializer,
  WriteHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t))
{
  return boost::beast::http::async_write(
    stream,
    serializer,
    std::forward<WriteHandler>(handler));
}

} // foxy

#endif // FOXY_SESSION_IMPL_HPP_
