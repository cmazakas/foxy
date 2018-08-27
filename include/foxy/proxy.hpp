#ifndef FOXY_PROXY_HPP_
#define FOXY_PROXY_HPP_

#include "foxy/multi_stream.hpp"
#include "foxy/log.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>

#include <boost/beast/http/error.hpp>

#include <memory>
#include <functional>

namespace foxy
{

struct proxy
  : boost::asio::coroutine
  , public std::enable_shared_from_this<proxy>
{
public:
  using acceptor_type = boost::asio::ip::tcp::acceptor;
  using endpoint_type = boost::asio::ip::tcp::endpoint;
  using stream_type   = multi_stream;
  using executor_type = stream_type::executor_type;

private:
  stream_type   stream_;
  acceptor_type acceptor_;

public:
  proxy()             = delete;
  proxy(proxy const&) = delete;
  proxy(proxy&&)      = default;

  explicit proxy(
    boost::asio::io_context& io,
    endpoint_type const&     endpoint,
    bool                     reuse_addr = false);

  auto get_executor() -> executor_type;

  auto async_accept(boost::system::error_code ec = {}) -> void;
};

} // foxy

#endif // FOXY_PROXY_HPP_