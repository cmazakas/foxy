//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#include <foxy/proxy.hpp>
#include <foxy/client_session.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include <boost/beast/http.hpp>

#include <memory>

#include <catch2/catch.hpp>

using boost::asio::ip::tcp;
namespace ip   = boost::asio::ip;
namespace asio = boost::asio;
namespace http = boost::beast::http;

TEST_CASE("Our forward proxy")
{
  std::cout << "starting proxy test suite now\n";

  SECTION("should reject all non-persistent connections")
  {
    asio::io_context io;

    auto was_valid_response = false;

    asio::spawn(
      [&](asio::yield_context yield)
      {
        auto const src_addr     = ip::make_address_v4("127.0.0.1");
        auto const src_port     = static_cast<unsigned short>(1337);
        auto const src_endpoint = tcp::endpoint(src_addr, src_port);

        auto const reuse_addr = true;

        auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr);
        proxy->async_accept();
        auto client = foxy::client_session(io);
        client.async_connect("127.0.0.1", "1337", yield);

        auto request = http::request<http::empty_body>(http::verb::get, "/", 11);
        request.keep_alive(false);

        http::response_parser<http::string_body> res_parser;

        client.async_request(request, res_parser, yield);

        auto ec = boost::system::error_code();
        client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
        client.stream.plain().close(ec);

        auto response = res_parser.release();

        auto const was_valid_result = response.result() == http::status::bad_request;
        auto const was_valid_body   = response.body() == "Connection must be persistent to allow proper tunneling\n\n";

        was_valid_response = was_valid_result && was_valid_body;
        proxy->cancel(ec);
        proxy.reset();
      });

    io.run();
    REQUIRE(was_valid_response);
  }

  SECTION("should reject all requests that aren't CONNECTs")
  {
    asio::io_context io;

    auto was_valid_response = false;

    asio::spawn(
      [&](asio::yield_context yield)
      {
        auto const src_addr     = ip::make_address_v4("127.0.0.1");
        auto const src_port     = static_cast<unsigned short>(1337);
        auto const src_endpoint = tcp::endpoint(src_addr, src_port);

        auto const reuse_addr = true;

        auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr);
        proxy->async_accept();
        auto client = foxy::client_session(io);
        client.async_connect("127.0.0.1", "1337", yield);

        auto request = http::request<http::empty_body>(http::verb::get, "/", 11);
        request.keep_alive(true);

        http::response_parser<http::string_body> res_parser;

        client.async_request(request, res_parser, yield);

        auto response = res_parser.release();

        auto const was_valid_result = response.result() == http::status::method_not_allowed;
        auto const was_valid_body   = response.body() == "Invalid request method. Only CONNECT is supported\n\n";

        auto ec = boost::system::error_code();
        client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
        client.stream.plain().close(ec);

        was_valid_response = was_valid_result && was_valid_body;
        proxy->cancel(ec);
      });

    io.run();
    REQUIRE(was_valid_response);
  }

  SECTION("should reject messages with payloads")
  {
    asio::io_context io;

    auto was_valid_response = false;

    asio::spawn(
      [&](asio::yield_context yield)
      {
        auto const src_addr     = ip::make_address_v4("127.0.0.1");
        auto const src_port     = static_cast<unsigned short>(1337);
        auto const src_endpoint = tcp::endpoint(src_addr, src_port);

        auto const reuse_addr = true;

        auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr);
        proxy->async_accept();
        auto client = foxy::client_session(io);
        client.async_connect("127.0.0.1", "1337", yield);

        auto request = http::request<http::string_body>(http::verb::get, "/", 11);
        request.body() = "ha ha ha, this is an non-zero length body";
        request.prepare_payload();

        http::response_parser<http::string_body> res_parser;

        client.async_request(request, res_parser, yield);

        auto response = res_parser.release();

        auto const was_valid_result = response.result() == http::status::bad_request;
        auto const was_valid_body   = response.body() == "Messages with bodies are not supported for establishing a tunnel\n\n";

        auto ec = boost::system::error_code();
        client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
        client.stream.plain().close(ec);

        was_valid_response = was_valid_result && was_valid_body;
        proxy->cancel(ec);
      });

    io.run();
    REQUIRE(was_valid_response);
  }
}
