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
#include <boost/asio/associated_allocator.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/experimental/test/stream.hpp>

#include <iostream>

#include <catch2/catch.hpp>

namespace net   = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;

using test_stream = beast::test::stream;

TEST_CASE("Our async HTTP relay")
{
  SECTION("should be able to relay messages from a server client to a remote")
  {
    net::io_context io;

    auto req_stream = test_stream(io);
    auto res_stream = test_stream(io);

    auto server_stream = test_stream(io);
    auto client_stream = test_stream(io);

    auto server = foxy::basic_session<test_stream>(std::move(server_stream));
    auto client = foxy::basic_session<test_stream>(std::move(client_stream));

    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);

    auto response = http::response<http::string_body>(
      http::status::ok, 11, "I bestow the heads of virgins and the first-born sons!!!!\n");

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

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

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
    net::io_context io;

    auto req_stream = test_stream(io);
    auto res_stream = test_stream(io);

    auto server_stream = test_stream(io);
    auto client_stream = test_stream(io);

    auto server = foxy::basic_session<test_stream>(std::move(server_stream));
    auto client = foxy::basic_session<test_stream>(std::move(client_stream));

    // in this test, it's our proxy client that initiates the teardown of the
    // Connection
    //
    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);
    request.keep_alive(false);
    request.insert(http::field::connection, "foo");

    auto response = http::response<http::string_body>(
      http::status::ok, 11, "I bestow the heads of virgins and the first-born sons!!!!\n");

    response.prepare_payload();

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    server.stream.plain().connect(res_stream);
    client.stream.plain().connect(req_stream);

    REQUIRE(req_stream.str() == "");
    REQUIRE(res_stream.str() == "");

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

    io.run();

    CHECK(req_stream.str() == "GET / HTTP/1.1\r\nConnection: close\r\n\r\n");

    CHECK(res_stream.str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 58\r\n"
          "Connection: close\r\n"
          "\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n");
  }

  SECTION("should forward a Connection: close if the remote sends one")
  {
    net::io_context io;

    auto req_stream = test_stream(io);
    auto res_stream = test_stream(io);

    auto server_stream = test_stream(io);
    auto client_stream = test_stream(io);

    auto server = foxy::basic_session<test_stream>(std::move(server_stream));
    auto client = foxy::basic_session<test_stream>(std::move(client_stream));

    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);

    auto response = http::response<http::string_body>(
      http::status::ok, 11, "I bestow the heads of virgins and the first-born sons!!!!\n");

    response.keep_alive(false);
    response.prepare_payload();

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    server.stream.plain().connect(res_stream);
    client.stream.plain().connect(req_stream);

    REQUIRE(req_stream.str() == "");
    REQUIRE(res_stream.str() == "");

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

    io.run();

    CHECK(req_stream.str() == "GET / HTTP/1.1\r\n\r\n");

    CHECK(res_stream.str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 58\r\n"
          "Connection: close\r\n"
          "\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n");
  }

  SECTION("should support requests with payloads")
  {
    net::io_context io;

    auto req_stream = test_stream(io);
    auto res_stream = test_stream(io);

    auto server_stream = test_stream(io);
    auto client_stream = test_stream(io);

    auto server = foxy::basic_session<test_stream>(std::move(server_stream));
    auto client = foxy::basic_session<test_stream>(std::move(client_stream));

    auto request = http::request<http::string_body>(
      http::verb::post, "/", 11,
      "Unholy Gravebirth is a good song but it can be a little off-putting "
      "when used in a test data\n");

    request.chunked(true);

    auto response = http::response<http::string_body>(
      http::status::ok, 11, "I bestow the heads of virgins and the first-born sons!!!!\n");

    response.chunked(true);

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    server.stream.plain().connect(res_stream);
    client.stream.plain().connect(req_stream);

    REQUIRE(req_stream.str() == "");
    REQUIRE(res_stream.str() == "");

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

    io.run();

    // this proves that our client instance successfully wrote the request to
    // the remote
    //
    CHECK(req_stream.str() ==
          "POST / HTTP/1.1\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
          "5d\r\n"
          "Unholy Gravebirth is a good song but it can be a little off-putting "
          "when used in a test data\n"
          "\r\n"
          "0\r\n\r\n");

    // this proves that our internal server instance successfully writes the
    // response back to the proxy client
    //
    CHECK(res_stream.str() ==
          "HTTP/1.1 200 OK\r\n"
          "Transfer-Encoding: chunked\r\n"
          "\r\n"
          "3a\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n"
          "\r\n"
          "0\r\n\r\n");
  }

  SECTION("should support relaying even when the header of the parser was already read in")
  {
    auto request = http::request<http::empty_body>(http::verb::get, "http://www.google.com", 11);
    request.set(http::field::host, "localhost");
    request.prepare_payload();

    auto response =
      http::response<http::string_body>(http::status::ok, 11, "google res goes here!");
    response.prepare_payload();

    net::io_context io;

    auto request_stream  = test_stream(io);
    auto response_stream = test_stream(io);

    auto client = foxy::basic_session<test_stream>(io);
    auto server = foxy::basic_session<test_stream>(io);

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    // the client writes the request to the stream
    // the server writes the response to the stream
    //
    client.stream.plain().connect(request_stream);
    server.stream.plain().connect(response_stream);

    net::spawn([&](net::yield_context yield) mutable {
      auto const allocator = net::get_associated_allocator(yield);

      http::request_parser<http::empty_body, std::decay_t<decltype(allocator)>> header_parser(
        std::piecewise_construct, std::make_tuple(), std::make_tuple(allocator));

      server.async_read_header(header_parser, yield);

      foxy::detail::async_relay(server, client, std::move(header_parser), yield);
    });

    io.run();

    CHECK(request_stream.str() == "GET http://www.google.com HTTP/1.1\r\nHost: localhost\r\n\r\n");
    CHECK(response_stream.str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 21\r\n\r\n"
          "google res goes here!");
  }
}
