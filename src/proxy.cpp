//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#include <foxy/proxy.hpp>
#include <foxy/server_session.hpp>
#include <foxy/client_session.hpp>
#include <foxy/log.hpp>
#include <foxy/utility.hpp>

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
struct async_connect_op
{
  struct state
  {
    // our session with the current client
    //
    foxy::server_session session;

    // our session with the client's intended remote
    //
    foxy::client_session client;

    // here we store an optional request parser so that we can recycle storage
    // while also adhering to the constraint that a new parser must be
    // constructed for each message we wish to parse
    //
    optional<http::request_parser<http::empty_body>> parser;

    // used by our initial session handling to let the client know what errors
    // occurred in establishing a connection with the intended remoted
    //
    http::response<http::string_body> err_response;

    // used by our initial session handling to let the client know that the
    // tunnel has been successfully established
    //
    http::response<http::empty_body> tunnel_res;

    // finite-sized buffer used for chunk-by-chunk transfer of the message from
    // the client to the remote
    //
    std::array<char, 4096> buf;

    // optional parsers and serializers that we can use to forward the client's
    // messages
    //
    optional<http::request_parser<http::buffer_body>>      request_buf_parser;
    optional<http::request_serializer<http::buffer_body>>  request_buf_sr;

    optional<http::response_parser<http::buffer_body>>     response_buf_parser;
    optional<http::response_serializer<http::buffer_body>> response_buf_sr;

    // the two main async operations that form our proxy server
    //
    boost::asio::coroutine connect_coro;
    boost::asio::coroutine tunnel_coro;

    state(foxy::multi_stream stream)
      : session(std::move(stream))
      , client(session.get_executor().context())
      , buf{ 0 }
      , request_buf_parser(boost::in_place_init)
      , request_buf_sr(request_buf_parser->get())
      , response_buf_parser(boost::in_place_init)
      , response_buf_sr(response_buf_parser->get())
    {
    }
  };

  std::unique_ptr<state> p_;

  async_connect_op(foxy::multi_stream stream);

  struct on_connect_t {};
  struct on_tunnel_t  {};
  struct on_relay_t   {};

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
  operator()(
    boost::system::error_code ec,
    std::size_t               bytes_transferred);

  void
  write_error(boost::beast::http::status const status, std::string what);
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
        foxy::log_error(ec, "foxy::proxy::async_accept");
        continue;
      }

      std::cout << "accepted connection\n";
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
write_error(boost::beast::http::status const status, std::string what)
{
  auto& s = *p_;

  s.err_response.result(status);

  s.err_response.body() = std::move(what);
  s.err_response.prepare_payload();

  s.session.async_write(s.err_response, std::move(*this));
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
  boost::system::error_code ec,
  std::size_t               bytes_transferred)
{
  using namespace std::placeholders;
  namespace beast = boost::beast;

  auto should_tunnel = false;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(s.connect_coro)
  {
    while (true) {
      if (s.parser) { s.parser = boost::none; }
      s.parser.emplace();

      BOOST_ASIO_CORO_YIELD
      s.session.async_read(*s.parser, std::move(*this));

      if (ec == http::error::end_of_stream) { break; }

      if (ec == http::error::unexpected_body) {
        BOOST_ASIO_CORO_YIELD
        write_error(
          http::status::bad_request,
          "Messages with bodies are not supported for establishing a tunnel\n\n");

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::unexpected_body::write_error");
          BOOST_ASIO_CORO_YIELD break;
        }

        s.session.buffer.consume(s.session.buffer.size());
        continue;
      }

      if (ec) {
        foxy::log_error(ec, "foxy::proxy::async_connect_op::async_read::read_error");
        BOOST_ASIO_CORO_YIELD break;
      }

      // we can only form a proper tunnel over a persistent connection
      //
      if (!s.parser->get().keep_alive()) {
        BOOST_ASIO_CORO_YIELD
        write_error(
          http::status::bad_request,
          "Connection must be persistent to allow proper tunneling\n\n");

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::non_keepalive_request::write_error");
          BOOST_ASIO_CORO_YIELD break;
        }

        break;
      }

      // a CONNECT must be used to signify tunnel semantics
      //
      if (s.parser->get().method() != http::verb::connect) {
        BOOST_ASIO_CORO_YIELD
        write_error(
          http::status::method_not_allowed,
          "Invalid request method. Only CONNECT is supported\n\n");

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::non_connect_verb::write_error");
          BOOST_ASIO_CORO_YIELD break;
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
          beast::bind_handler(std::move(*this), on_connect_t{}, _1, _2));
      }

      // TODO: support 504 for `operation_aborted` error code
      //
      if (ec) {
        BOOST_ASIO_CORO_YIELD
        write_error(
          http::status::bad_request,
          std::string("Unable to establish connection with the remote\n") +
          "Error: " +
          ec.message() + "\n\n");

        s.err_response = {};
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::async_accept_op::failed_connect::write_error");
          BOOST_ASIO_CORO_YIELD break;
        }

        continue;
      }

      BOOST_ASIO_CORO_YIELD
      s.session.async_write(s.tunnel_res, std::move(*this));
      if (ec) {
        foxy::log_error(ec, "foxy::proxy::async_connect_op::failed_write_back_successful_tunnel_res");
        BOOST_ASIO_CORO_YIELD break;
      }

      // at this point, we can safely use our `client_session` object for
      // tunneling requests from the client to the upstream
      //
      should_tunnel = true;
      BOOST_ASIO_CORO_YIELD break;
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
    s.session.stream.plain().shutdown(tcp::socket::shutdown_send, ec);

    if (s.parser) { s.parser = boost::none; }
    s.parser.emplace();

    BOOST_ASIO_CORO_YIELD
    s.session.async_read(*s.parser, std::move(*this));

    if (ec) {
      foxy::log_error(ec, "foxy::proxy::tunnel::shutdown_wait_for_eof_error");
    }

    s.session.stream.plain().shutdown(tcp::socket::shutdown_receive, ec);
    s.session.stream.plain().close(ec);
  }

  if (!s.connect_coro.is_complete() || ec || !should_tunnel) { return; }
  (*this)(on_tunnel_t{}, {}, 0);
}



void
async_connect_op::
operator()(
  on_tunnel_t,
  boost::system::error_code ec,
  std::size_t               bytes_transferred)
{
  using namespace std::placeholders;
  namespace beast = boost::beast;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(s.tunnel_coro)
  {
    BOOST_ASIO_CORO_YIELD
    s.session.async_read_header(
      *s.request_buf_parser,
      beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

    // TODO: close connection with end server here
    //
    if (ec) {
      foxy::log_error(ec, "foxy::proxy::tunnel::reading_client_header");
      return;
    }

    BOOST_ASIO_CORO_YIELD
    s.client.async_write_header(
      *s.request_buf_sr,
      beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

    // TODO: make sure that we terminate the connection with our `session` here
    //
    if (ec) {
      foxy::log_error(ec, "foxy::proxy::tunnel:writing_client_request_header");
      return;
    }

    do
    {
      if (!s.request_buf_parser->is_done()) {
        s.request_buf_parser->get().body().data = s.buf.data();
        s.request_buf_parser->get().body().size = s.buf.size();

        BOOST_ASIO_CORO_YIELD
        s.session.async_read(
          *s.request_buf_parser,
          beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

        if (ec == boost::beast::http::error::need_buffer) {
          ec = {};
        }

        // TODO: close connection with end server here
        //
        if (ec) {
          foxy::log_error(ec, "foxy::proxy::tunnel::read_session_body_chunk");
          return;
        }

        s.request_buf_parser->get().body().size = s.buf.size() - s.request_buf_parser->get().body().size;
        s.request_buf_parser->get().body().data = s.buf.data();
        s.request_buf_parser->get().body().more = !s.request_buf_parser->is_done();

      } else {

        s.request_buf_parser->get().body().data = nullptr;
        s.request_buf_parser->get().body().size = 0;
      }

      BOOST_ASIO_CORO_YIELD
      s.client.async_write(
        *s.request_buf_sr,
        beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

      if (ec == boost::beast::http::error::need_buffer) {
          ec = {};
      }

      // TODO: close connection with the main session here
      if (ec) {
        return;
      }
    } while(!s.request_buf_parser->is_done() && !s.request_buf_sr->is_done());

    BOOST_ASIO_CORO_YIELD
    s.client.async_read_header(
      *s.response_buf_parser,
      beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

    if (ec) {
      std::cout << ec.message() << "\n";
    }

    BOOST_ASIO_CORO_YIELD
    s.session.async_write_header(
      *s.response_buf_sr,
      beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

    if (ec) {
      std::cout << ec.message() << "\n";;
    }

    do
    {
      if (!(*s.response_buf_parser).is_done()) {
        (*s.response_buf_parser).get().body().data = s.buf.data();
        (*s.response_buf_parser).get().body().size = s.buf.size();

        BOOST_ASIO_CORO_YIELD
        s.client.async_read(
          *s.response_buf_parser,
          beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

        if (ec == boost::beast::http::error::need_buffer) {
          ec = {};
        }

        if (ec) {
          std::cout << ec.message() << "\n";
          return;
        }

        (*s.response_buf_parser).get().body().size = s.buf.size() - (*s.response_buf_parser).get().body().size;
        (*s.response_buf_parser).get().body().data = s.buf.data();
        (*s.response_buf_parser).get().body().more = !(*s.response_buf_parser).is_done();

      } else {

        (*s.response_buf_parser).get().body().data = nullptr;
        (*s.response_buf_parser).get().body().size = 0;
      }

      std::cout << "writing back body chunk...\n";
      // Write everything in the buffer (which might be empty)
      BOOST_ASIO_CORO_YIELD
      s.session.async_write(
        *s.response_buf_sr,
        beast::bind_handler(std::move(*this), on_tunnel_t{}, _1, _2));

      // This error is returned when buffer_body uses up the buffer
      if (ec == boost::beast::http::error::need_buffer) {
          ec = {};
      }

      if (ec) {
        return;
      }
    } while(!(*s.response_buf_parser).is_done() && !(*s.response_buf_sr).is_done());
  }
}

}
