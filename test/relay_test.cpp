//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/session.hpp>
#include <foxy/detail/relay.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/experimental/test/stream.hpp>

#include <boost/beast/core/ostream.hpp>

#include <catch2/catch.hpp>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;

using stream_type = beast::test::stream;

TEST_CASE("Our async HTTP relay")
{
  SECTION("should be able to relay messages from a server client to a remote")
  {
    asio::io_context io;

    auto req_stream = stream_type(io);
    auto res_stream = stream_type(io);

    auto server_stream = stream_type(io);
    auto client_stream = stream_type(io);

    auto server = foxy::basic_session<stream_type>(std::move(server_stream));
    auto client = foxy::basic_session<stream_type>(std::move(client_stream));

    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);

    auto response = http::response<http::string_body>(
      http::status::ok, 11,
      "I bestow the heads of virgins and the first-born sons!!!!\n");

    response.prepare_payload();

    // server reads the proxy client's request
    //
    beast::ostream(server.stream.plain().buffer()) << request;

    // our internal proxy's client reads in the response from the remote
    //
    beast::ostream(client.stream.plain().buffer()) << response;

    // our internal server session is what writes the response back to the
    // proxy client
    //
    server.stream.plain().connect(res_stream);

    // our internal client is what writes the request from the proxy client
    // to the end remote
    //
    client.stream.plain().connect(req_stream);

    REQUIRE(req_stream.str() == "");
    REQUIRE(res_stream.str() == "");

    asio::spawn([&](asio::yield_context yield) mutable {
      foxy::detail::async_relay(server, client, yield);
    });

    io.run();

    // this proves that our client instance successfully wrote the request to
    // the remote
    //
    CHECK(req_stream.str() == "GET / HTTP/1.1\r\n\r\n");

    // this proves that our internal server instance successfully writes the
    // response back to the proxy client
    //
    CHECK(res_stream.str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 58\r\n\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n");
  }

  SECTION("should relay while respecting Connection header semantics")
  {
    asio::io_context io;

    auto req_stream = stream_type(io);
    auto res_stream = stream_type(io);

    auto server_stream = stream_type(io);
    auto client_stream = stream_type(io);

    auto server = foxy::basic_session<stream_type>(std::move(server_stream));
    auto client = foxy::basic_session<stream_type>(std::move(client_stream));

    // in this test, it's our proxy client that initiates the teardown of the
    // Connection
    //
    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);
    request.keep_alive(false);

    auto response = http::response<http::string_body>(
      http::status::ok, 11,
      "I bestow the heads of virgins and the first-born sons!!!!\n");

    response.prepare_payload();

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    server.stream.plain().connect(res_stream);
    client.stream.plain().connect(req_stream);

    REQUIRE(req_stream.str() == "");
    REQUIRE(res_stream.str() == "");

    asio::spawn([&](asio::yield_context yield) mutable {
      foxy::detail::async_relay(server, client, yield);
    });

    io.run();

    CHECK(req_stream.str() == "GET / HTTP/1.1\r\nConnection: close\r\n\r\n");
    CHECK(res_stream.str() ==
          "HTTP/1.1 200 OK\r\n"
          "Connection: close\r\n"
          "Content-Length: 58\r\n"
          "\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n");
  }
}
