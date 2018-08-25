#include "foxy/proxy.hpp"
#include "foxy/server_session.hpp"
#include "foxy/log.hpp"

#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/status.hpp>

#include <boost/asio/ip/tcp.hpp>

#include <boost/optional/optional.hpp>

#include <memory>

using boost::optional;
using boost::asio::ip::tcp;
namespace http = boost::beast::http;

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

namespace
{
struct async_connect_op : boost::asio::coroutine
{
  struct state
  {
    foxy::server_session session;

    // here we store an optional request parser so that we can recycle storage
    // while also adhering to the constratin that a new parser must be
    // constructed for each message
    //
    optional<http::request_parser<http::empty_body>> parser;
    http::response<http::string_body>                err_response;

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
    while (true) {
      if (s.parser) {
        s.parser = boost::none;
      }
      s.parser.emplace();

      BOOST_ASIO_CORO_YIELD
      s.session.async_read_header(*s.parser, std::move(*this));

      // a CONNECT must be used to signify tunnel semantics
      //
      if (s.parser->get().method() != http::verb::connect) {
        s.err_response.result(http::status::method_not_allowed);
        s.err_response.body() = "Invalid request method. Only CONNECT is supported\n\n";
        s.err_response.prepare_payload();

        BOOST_ASIO_CORO_YIELD
        s.session.async_write(s.err_response, std::move(*this));

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::non_connect_verb");
          break;
        }

        continue;
      }

      // we can only form a proper tunnel over a persistent connection
      //
      if (!s.parser->get().keep_alive()) {
        s.err_response.result(http::status::bad_request);
        s.err_response.body() = "Connection must be persistent to allow proper tunneling\n\n";
        s.err_response.prepare_payload();

        BOOST_ASIO_CORO_YIELD
        s.session.async_write(s.err_response, std::move(*this));

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::non_keepalive_request");
          break;
        }

        s.session.stream.tcp().shutdown(tcp::socket::shutdown_both);

        break;
      }
    }
  }
}

}
