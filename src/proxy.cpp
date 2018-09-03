#include "foxy/proxy.hpp"
#include "foxy/server_session.hpp"
#include "foxy/client_session.hpp"
#include "foxy/log.hpp"
#include "foxy/utility.hpp"

#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/error.hpp>

#include <boost/optional/optional.hpp>

#include <memory>
#include <iostream>

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

auto foxy::proxy::cancel(boost::system::error_code& ec) -> void
{
  acceptor_.cancel(ec);
}

namespace
{
struct async_connect_op : boost::asio::coroutine
{
  struct state
  {
    foxy::server_session session;
    foxy::client_session client;

    // here we store an optional request parser so that we can recycle storage
    // while also adhering to the constratin that a new parser must be
    // constructed for each message
    //
    optional<http::request_parser<http::empty_body>> parser;
    http::response<http::string_body>                err_response;

    state(foxy::multi_stream stream)
    : session(std::move(stream))
    , client(session.get_executor().context())
    {
    }
  };

  std::unique_ptr<state> p_;

  async_connect_op(foxy::multi_stream stream);

  struct on_connect_t {};
  struct on_tunnel_t {};

  void
  operator()(
    on_connect_t,
    boost::system::error_code      ec,
    boost::asio::ip::tcp::endpoint endpoint);

  void
  operator()(
    on_tunnel_t,
    boost::system::error_code ec,
    std::size_t               bytes_transferred);

  void
  operator()(boost::system::error_code ec, std::size_t bytes_transferred);
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
        stream_.plain(),
        std::bind(&proxy::async_accept, shared_from_this(), _1));

      if (ec == boost::asio::error::operation_aborted) {
        break;
      }

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

void
async_connect_op::
operator()(
  on_connect_t,
  boost::system::error_code      ec,
  boost::asio::ip::tcp::endpoint endpoint)
{
  (*this)(ec, 0);
}

void
async_connect_op::
operator()(
  on_tunnel_t,
  boost::system::error_code ec,
  std::size_t               bytes_transferred)
{

}

void
async_connect_op::
operator()(boost::system::error_code ec, std::size_t bytes_transferred)
{
  using namespace std::placeholders;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    while (true) {
      // A new instance of the parser is required for each message.
      //
      if (s.parser) {
        s.parser = boost::none;
      }
      s.parser.emplace();

      std::cout << "buffer size before reading: " << s.session.buffer.size() << "\n";

      BOOST_ASIO_CORO_YIELD
      s.session.async_read(*s.parser, std::move(*this));

      std::cout << "buffer size after reading: " << s.session.buffer.size() << "\n";

      if (ec == http::error::unexpected_body) {
        std::cout << "encountered unexpected message body\n";

        s.err_response.result(http::status::bad_request);
        s.err_response.body() = "Messages with bodies are not supported for establishing a tunnel\n\n";
        s.err_response.prepare_payload();

        BOOST_ASIO_CORO_YIELD
        s.session.async_write(s.err_response, std::move(*this));

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::unexpected_body::write_error");
          return;
        }

        s.session.buffer.consume(s.session.buffer.size());

        continue;
      }

      if (ec == http::error::end_of_stream) {
        std::cout << "received the end of the stream\n";
        break;
      }

      if (ec) {
        foxy::log_error(ec, "foxy::proxy::async_connect_op::async_read::read_error");
        return;
      }

      // we can only form a proper tunnel over a persistent connection
      //
      if (!s.parser->get().keep_alive()) {
        s.err_response.result(http::status::bad_request);
        s.err_response.body() = "Connection must be persistent to allow proper tunneling\n\n";
        s.err_response.prepare_payload();

        BOOST_ASIO_CORO_YIELD s.session.async_write(s.err_response, std::move(*this));

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::non_keepalive_request::write_error");
          return;
        }

        std::cout << "non-persistent connection, going to break session loop now...\n";
        break;
      }

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
          foxy::log_error(ec, "foxy::proxy::async_accept_op::non_connect_verb::write_error");
          return;
        }

        continue;
      }

      // extract request target and attempt to form the tunnel
      //
      BOOST_ASIO_CORO_YIELD
      {
        auto request = s.parser->release();
        auto target  = request.target();

        auto host_and_port = foxy::parse_authority_form(target);

        s.client.async_connect(
          std::move(std::get<0>(host_and_port)),
          std::move(std::get<1>(host_and_port)),
          boost::beast::bind_handler(std::move(*this), on_connect_t{}, _1, _2));
      }

      // TODO: support 504 for `operation_aborted` error code
      //
      if (ec) {
        s.err_response.result(http::status::bad_request);
        s.err_response.body() =
          "Unable to establish connection with the remote\n";

        s.err_response.body() += "Error: " + ec.message() + "\n\n";

        s.err_response.prepare_payload();

        BOOST_ASIO_CORO_YIELD
        s.session.async_write(s.err_response, std::move(*this));

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::failed_connect::write_error");
          return;
        }

        continue;
      }

      // at this point, we can safely use our `client_session` object for
      // tunneling requests from the client to the upstream
      //
    }

    // http rfc 7230 section 6.6 Tear-down
    // -----------------------------------
    // To avoid the TCP reset problem, servers typically close a connection
    // in stages.  First, the server performs a half-close by closing only
    // the write side of the read/write connection.  The server then
    // continues to read from the connection until it receives a
    // corresponding close by the client, or until the server is reasonably
    // certain that its own TCP stack has received the client's
    // acknowledgement of the packet(s) containing the server's last
    // response.  Finally, the server fully closes the connection.
    //
    std::cout << "at shutdown portion\n";
    s.session.stream.plain().shutdown(tcp::socket::shutdown_send, ec);

    if (s.parser) {
      s.parser = boost::none;
    }
    s.parser.emplace();

    BOOST_ASIO_CORO_YIELD
    s.session.async_read(*s.parser, std::move(*this));

    if (ec) {
      std::cout << ec << "\n";
    }

    s.session.stream.plain().shutdown(tcp::socket::shutdown_receive, ec);
    s.session.stream.plain().close(ec);

    std::cout << "done closing the socket\n\n";
  }
}

}
