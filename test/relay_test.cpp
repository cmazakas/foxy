//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include <boost/beast/_experimental/test/stream.hpp>

#include <catch2/catch.hpp>

namespace net   = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;

using test_stream = boost::beast::test::stream;

TEST_CASE("relay_test")
{
  SECTION("should be able to relay messages from a server client to a remote")
  {
    net::io_context io;

    auto req_stream = foxy::basic_multi_stream<test_stream>(io);
    auto res_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server_stream = foxy::basic_multi_stream<test_stream>(io);
    auto client_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server = foxy::basic_session<test_stream, boost::beast::flat_buffer>(
      std::move(server_stream), foxy::session_opts{});

    auto client = foxy::basic_session<test_stream, boost::beast::flat_buffer>(
      std::move(client_stream), foxy::session_opts{});

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
    server.stream.plain().connect(res_stream.plain());

    // our internal client is what writes the request from the proxy client
    // to the end remote
    //
    client.stream.plain().connect(req_stream.plain());

    REQUIRE(req_stream.plain().str() == "");
    REQUIRE(res_stream.plain().str() == "");

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

    io.run();

    // this proves that our client instance successfully wrote the request to
    // the remote
    //
    CHECK(req_stream.plain().str() == "GET / HTTP/1.1\r\nVia: 1.1 foxy\r\n\r\n");

    // this proves that our internal server instance successfully writes the
    // response back to the proxy client
    //
    CHECK(res_stream.plain().str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 58\r\n"
          "Via: 1.1 foxy\r\n"
          "\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n");
  }

  SECTION("should relay while respecting Connection header semantics")
  {
    net::io_context io;

    auto req_stream = foxy::basic_multi_stream<test_stream>(io);
    auto res_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server_stream = foxy::basic_multi_stream<test_stream>(io);
    auto client_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(server_stream), {});

    auto client =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(client_stream), {});

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

    server.stream.plain().connect(res_stream.plain());
    client.stream.plain().connect(req_stream.plain());

    REQUIRE(req_stream.plain().str() == "");
    REQUIRE(res_stream.plain().str() == "");

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

    io.run();

    CHECK(req_stream.plain().str() ==
          "GET / HTTP/1.1\r\nConnection: close\r\nVia: 1.1 foxy\r\n\r\n");

    CHECK(res_stream.plain().str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 58\r\n"
          "Connection: close\r\n"
          "Via: 1.1 foxy\r\n"
          "\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n");
  }

  SECTION("should forward a Connection: close if the remote sends one")
  {
    net::io_context io;

    auto req_stream = foxy::basic_multi_stream<test_stream>(io);
    auto res_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server_stream = foxy::basic_multi_stream<test_stream>(io);
    auto client_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(server_stream), {});

    auto client =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(client_stream), {});

    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);

    auto response = http::response<http::string_body>(
      http::status::ok, 11, "I bestow the heads of virgins and the first-born sons!!!!\n");

    response.keep_alive(false);
    response.prepare_payload();

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    server.stream.plain().connect(res_stream.plain());
    client.stream.plain().connect(req_stream.plain());

    REQUIRE(req_stream.plain().str() == "");
    REQUIRE(res_stream.plain().str() == "");

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

    io.run();

    CHECK(req_stream.plain().str() == "GET / HTTP/1.1\r\nVia: 1.1 foxy\r\n\r\n");

    CHECK(res_stream.plain().str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 58\r\n"
          "Connection: close\r\n"
          "Via: 1.1 foxy\r\n"
          "\r\n"
          "I bestow the heads of virgins and the first-born sons!!!!\n");
  }

  SECTION("should support requests with payloads")
  {
    net::io_context io;

    auto req_stream = foxy::basic_multi_stream<test_stream>(io);
    auto res_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server_stream = foxy::basic_multi_stream<test_stream>(io);
    auto client_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(server_stream), {});
    auto client =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(client_stream), {});

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

    server.stream.plain().connect(res_stream.plain());
    client.stream.plain().connect(req_stream.plain());

    REQUIRE(req_stream.plain().str() == "");
    REQUIRE(res_stream.plain().str() == "");

    net::spawn(
      [&](net::yield_context yield) mutable { foxy::detail::async_relay(server, client, yield); });

    io.run();

    // this proves that our client instance successfully wrote the request to
    // the remote
    //
    CHECK(req_stream.plain().str() ==
          "POST / HTTP/1.1\r\n"
          "Transfer-Encoding: chunked\r\n"
          "Via: 1.1 foxy\r\n"
          "\r\n"
          "5d\r\n"
          "Unholy Gravebirth is a good song but it can be a little off-putting "
          "when used in a test data\n"
          "\r\n"
          "0\r\n\r\n");

    // this proves that our internal server instance successfully writes the
    // response back to the proxy client
    //
    CHECK(res_stream.plain().str() ==
          "HTTP/1.1 200 OK\r\n"
          "Transfer-Encoding: chunked\r\n"
          "Via: 1.1 foxy\r\n"
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

    auto response =
      http::response<http::string_body>(http::status::ok, 11, "google res goes here!");

    response.prepare_payload();

    net::io_context io;

    auto request_stream  = foxy::basic_multi_stream<test_stream>(io);
    auto response_stream = foxy::basic_multi_stream<test_stream>(io);

    auto client = foxy::basic_session<test_stream, boost::beast::flat_buffer>(io, {});
    auto server = foxy::basic_session<test_stream, boost::beast::flat_buffer>(io, {});

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    // the client writes the request to the stream
    // the server writes the response to the stream
    //
    client.stream.plain().connect(request_stream.plain());
    server.stream.plain().connect(response_stream.plain());

    net::spawn([&](net::yield_context yield) mutable {
      http::request_parser<http::empty_body> header_parser;

      server.async_read_header(header_parser, yield);

      foxy::detail::async_relay(server, client, std::move(header_parser), yield);
    });

    io.run();

    CHECK(request_stream.plain().str() ==
          "GET http://www.google.com HTTP/1.1\r\nHost: localhost\r\nVia: 1.1 foxy\r\n\r\n");
    CHECK(response_stream.plain().str() ==
          "HTTP/1.1 200 OK\r\n"
          "Content-Length: 21\r\n"
          "Via: 1.1 foxy\r\n"
          "\r\n"
          "google res goes here!");
  }

  SECTION("should signal to close the tunnel for a potential request loop attack (request version)")
  {
    net::io_context io;

    auto req_stream = foxy::basic_multi_stream<test_stream>(io);
    auto res_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server_stream = foxy::basic_multi_stream<test_stream>(io);
    auto client_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(server_stream), {});
    auto client =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(client_stream), {});

    auto fields = http::fields();
    fields.insert(http::field::via, "1.1 foxy");

    auto request = http::request<http::string_body>(
      http::verb::post, "/", 11,
      "Unholy Gravebirth is a good song but it can be a little off-putting "
      "when used in a test data\n",
      fields);

    request.prepare_payload();

    auto response = http::response<http::string_body>(
      http::status::ok, 11, "I bestow the heads of virgins and the first-born sons!!!!\n");

    response.chunked(true);

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    server.stream.plain().connect(res_stream.plain());
    client.stream.plain().connect(req_stream.plain());

    REQUIRE(req_stream.plain().str() == "");
    REQUIRE(res_stream.plain().str() == "");

    auto close_tunnel = false;

    net::spawn([&](net::yield_context yield) mutable {
      close_tunnel = foxy::detail::async_relay(server, client, yield);
    });

    io.run();

    CHECK(close_tunnel);
    CHECK(req_stream.plain().str() == "");
    CHECK(res_stream.plain().str() == "");
  }

  SECTION(
    "should signal to close the tunnel for a potential request loop attack (response version)")
  {
    net::io_context io;

    auto req_stream = foxy::basic_multi_stream<test_stream>(io);
    auto res_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server_stream = foxy::basic_multi_stream<test_stream>(io);
    auto client_stream = foxy::basic_multi_stream<test_stream>(io);

    auto server =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(server_stream), {});

    auto client =
      foxy::basic_session<test_stream, boost::beast::flat_buffer>(std::move(client_stream), {});

    auto req_fields = http::fields();
    req_fields.insert(http::field::via, "1.1 otherserver");

    auto request = http::request<http::string_body>(
      http::verb::post, "/", 11,
      "Unholy Gravebirth is a good song but it can be a little off-putting "
      "when used in a test data\n",
      req_fields);

    request.prepare_payload();

    auto fields = http::fields();
    fields.insert(http::field::via, "1.1 someserver, 1.1 foxy, 1.0 anotherserver");

    auto response = http::response<http::string_body>(
      http::status::ok, 11, "I bestow the heads of virgins and the first-born sons!!!!\n", fields);

    response.chunked(true);

    beast::ostream(server.stream.plain().buffer()) << request;
    beast::ostream(client.stream.plain().buffer()) << response;

    server.stream.plain().connect(res_stream.plain());
    client.stream.plain().connect(req_stream.plain());

    REQUIRE(req_stream.plain().str() == "");
    REQUIRE(res_stream.plain().str() == "");

    auto close_tunnel = false;

    net::spawn([&](net::yield_context yield) mutable {
      close_tunnel = foxy::detail::async_relay(server, client, yield);
    });

    io.run();

    CHECK(close_tunnel);
    CHECK(req_stream.plain().str() ==
          "POST / HTTP/1.1\r\n"
          "Via: 1.1 otherserver\r\n"
          "Via: 1.1 foxy\r\n"
          "Content-Length: 93\r\n"
          "\r\n"
          "Unholy Gravebirth is a good song but it can be a little off-putting "
          "when used in a test data\n");
    CHECK(res_stream.plain().str() == "");
  }
}
