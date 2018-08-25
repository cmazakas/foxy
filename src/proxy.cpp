#include "foxy/proxy.hpp"
#include "foxy/server_session.hpp"

#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/empty_body.hpp>

#include <memory>

namespace
{
struct async_connect_op : boost::asio::coroutine
{
  struct state
  {
    foxy::server_session session;

    boost::beast::http::request_parser<
      boost::beast::http::empty_body
    > parser;

    state(foxy::multi_stream stream)
    : session(std::move(stream))
    {
    }
  };

  std::unique_ptr<state> p_;

  async_connect_op(foxy::multi_stream stream);
  void operator()(boost::system::error_code ec, std::size_t bytes_transferred);
};
}

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

      async_connect_op(std::move(stream_))({}, 0);
    }
  }
}

namespace
{

async_connect_op::async_connect_op(foxy::multi_stream stream)
: p_(std::make_unique<state>(std::move(stream)))
{
}

void async_connect_op::
operator()(boost::system::error_code ec, std::size_t bytes_transferred)
{
  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    BOOST_ASIO_CORO_YIELD
    s.session.async_read_header(s.parser, std::move(*this));
  }
}

}
