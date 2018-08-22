#include "foxy/proxy.hpp"

foxy::proxy::proxy(
  boost::asio::io_context& io,
  endpoint_type const&     endpoint,
  bool                     reuse_addr)
: stream_(io)
, acceptor_(io, endpoint, reuse_addr)
{
}

auto foxy::proxy::get_executor() -> executor_type
{
  return stream_.get_executor();
}

auto foxy::proxy::async_accept(boost::system::error_code ec) -> void
{
  using namespace std::placeholders;

  BOOST_ASIO_CORO_REENTER(*this)
  {
    for (;;) {
      BOOST_ASIO_CORO_YIELD
      acceptor_.async_accept(
        stream_.tcp(),
        std::bind(&proxy::async_accept, shared_from_this(), _1));

      if (ec) {
        log_error(ec, "foxy::proxy::async_accept");
        continue;
      }

      // TODO: create server_session object and then make sure it runs
      // asynchronously from there
      //
      // server_session session(std::move(stream_));
    }
  }
}