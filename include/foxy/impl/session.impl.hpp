#ifndef FOXY_SESSION_IMPL_HPP_
#define FOXY_SESSION_IMPL_HPP_

#include "foxy/session.hpp"

namespace foxy
{

template <class Parser, class ReadHandler>
auto
session::async_read_header(
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

template <class Parser, class ReadHandler>
auto
session::async_read(
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

template <class Serializer, class WriteHandler>
auto
session::async_write_header(
  Serializer&    serializer,
  WriteHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t))
{
  return boost::beast::http::async_write_header(
    stream,
    buffer,
    serializer,
    std::forward<ReadHandler>(handler));
}

template <class Serializer, class WriteHandler>
auto
session::async_write(
  Serializer&    serializer,
  WriteHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(WriteHandler, void(boost::system::error_code, std::size_t))
{
  return boost::beast::http::async_write(
    stream,
    buffer,
    serializer,
    std::forward<ReadHandler>(handler));
}

} // foxy

#endif // FOXY_SESSION_IMPL_HPP_
