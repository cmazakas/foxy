//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
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
namespace ssl  = boost::asio::ssl;

using namespace std::chrono_literals;

TEST_CASE("proxy_test")
{
  SECTION("should reject ill-formed requests")
  {
    asio::io_context io{1};

    auto was_valid_response = false;

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) {
      auto const addr     = ip::make_address_v4("127.0.0.1");
      auto const port     = static_cast<unsigned short>(1337);
      auto const endpoint = tcp::endpoint(addr, port);

      auto const reuse_addr = true;

      auto const request =
        http::request<http::empty_body>(http::verb::get, "lol-some-garbage-target", 11);

      auto proxy = std::make_shared<foxy::proxy>(io, endpoint, reuse_addr);
      proxy->async_accept();

      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 30s;
      client.async_connect("127.0.0.1", "1337", yield);

      http::response_parser<http::string_body> parser;
      client.async_request(request, parser, yield);

      auto ec = boost::system::error_code();
      client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
      client.stream.plain().close(ec);

      auto response = parser.release();

      auto const was_valid_result = response.result() == http::status::bad_request;
      auto const was_valid_body =
        response.body() ==
        "Malformed client request. Use either CONNECT <authority-uri> or <verb> <absolute-uri>";

      was_valid_response = was_valid_result && was_valid_body;
      proxy->cancel();
      proxy.reset();
    });

    io.run();
    REQUIRE(was_valid_response);
  }

  SECTION("should act as a relay for absolute URIs")
  {
    asio::io_context io{1};

    auto was_valid_response = false;

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) {
      auto const addr     = ip::make_address_v4("127.0.0.1");
      auto const port     = static_cast<unsigned short>(1337);
      auto const endpoint = tcp::endpoint(addr, port);

      auto const reuse_addr = true;

      auto const request =
        http::request<http::empty_body>(http::verb::get, "http://www.google.com:80/", 11);

      auto proxy = std::make_shared<foxy::proxy>(io, endpoint, reuse_addr);
      proxy->async_accept();

      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 30s;
      client.async_connect("127.0.0.1", "1337", yield);

      http::response_parser<http::string_body> parser;
      client.async_request(request, parser, yield);

      auto ec = boost::system::error_code();
      client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
      client.stream.plain().close(ec);

      auto response = parser.release();

      auto const was_valid_result = response.result() == http::status::ok;
      auto const was_valid_body   = response.body().size() > 0;

      was_valid_response = was_valid_result && was_valid_body;
      proxy->cancel();
      proxy.reset();
    });

    io.run();
    REQUIRE(was_valid_response);
  }

  SECTION("should relay via CONNECT")
  {
    asio::io_context io{1};

    auto was_valid_response = false;

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) {
      auto const addr     = ip::make_address_v4("127.0.0.1");
      auto const port     = static_cast<unsigned short>(1337);
      auto const endpoint = tcp::endpoint(addr, port);

      auto const reuse_addr = true;

      auto proxy = std::make_shared<foxy::proxy>(io, endpoint, reuse_addr);
      proxy->async_accept();

      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 30s;
      client.async_connect("127.0.0.1", "1337", yield);

      auto const request =
        http::request<http::empty_body>(http::verb::connect, "www.google.com:80", 11);

      http::response_parser<http::empty_body> tunnel_parser;
      tunnel_parser.skip(true);

      auto const google_request = http::request<http::empty_body>(http::verb::get, "/", 11);

      http::response_parser<http::string_body> google_parser;

      client.async_request(request, tunnel_parser, yield);
      client.async_request(google_request, google_parser, yield);

      auto ec = boost::system::error_code();
      client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
      client.stream.plain().close(ec);

      auto response = google_parser.release();

      auto const was_valid_result = response.result() == http::status::ok;
      auto const was_valid_body   = response.body().size() > 0;

      was_valid_response = was_valid_result && was_valid_body;
      proxy->cancel();
      proxy.reset();
    });

    io.run();
    REQUIRE(was_valid_response);
  }

  SECTION("should reject all non-persistent connections")
  {
    asio::io_context io{1};

    auto was_valid_response = false;

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) {
      auto const src_addr     = ip::make_address_v4("127.0.0.1");
      auto const src_port     = static_cast<unsigned short>(1337);
      auto const src_endpoint = tcp::endpoint(src_addr, src_port);

      auto const reuse_addr = true;

      auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr);
      proxy->async_accept();

      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 30s;
      client.async_connect("127.0.0.1", "1337", yield);

      auto request = http::request<http::empty_body>(http::verb::connect, "www.google.com:80", 11);
      request.keep_alive(false);

      http::response_parser<http::string_body> res_parser;

      client.async_request(request, res_parser, yield);

      auto ec = boost::system::error_code();
      client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
      client.stream.plain().close(ec);

      auto response = res_parser.release();

      auto const was_valid_result = response.result() == http::status::bad_request;
      auto const was_valid_body =
        response.body() == "CONNECT semantics require a persistent connection\n\n";

      was_valid_response = was_valid_result && was_valid_body;
      proxy->cancel();
      proxy.reset();
    });

    io.run();
    REQUIRE(was_valid_response);
  }

  SECTION("should forward a message over an encrypted connection")
  {
    asio::io_context io{1};

    auto was_valid_response = false;

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) {
      auto const src_addr     = ip::make_address_v4("127.0.0.1");
      auto const src_port     = static_cast<unsigned short>(1337);
      auto const src_endpoint = tcp::endpoint(src_addr, src_port);

      auto const reuse_addr = true;

      auto ctx = foxy::make_ssl_ctx(ssl::context::method::tlsv12_client);
      ctx.load_verify_file("root-cas.pem");

      auto opts = foxy::session_opts{ctx, 5s};

      auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr, opts);
      proxy->async_accept();

      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 30s;
      client.async_connect("127.0.0.1", "1337", yield);

      auto request = http::request<http::empty_body>(http::verb::connect, "www.google.com:443", 11);
      request.prepare_payload();

      http::response_parser<http::string_body> res_parser;
      res_parser.skip(true);

      auto request2 = http::request<http::empty_body>(http::verb::get, "/", 11);
      request2.prepare_payload();

      http::response_parser<http::string_body> res_parser2;

      client.async_request(request, res_parser, yield);

      client.async_request(request2, res_parser2, yield);

      auto response = res_parser2.release();

      auto const was_valid_result = (response.result() == http::status::ok);
      auto const was_valid_body =
        (response.body().size() > 0) && boost::string_view(response.body()).ends_with("</html>");

      auto ec = boost::system::error_code();
      client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
      client.stream.plain().close(ec);

      was_valid_response = was_valid_result && was_valid_body;
      proxy->cancel();
    });

    io.run();
    REQUIRE(was_valid_response);
  }

  SECTION("should forward a one-time relay over an encrypted connection")
  {
    asio::io_context io{1};

    auto was_valid_response = false;

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) {
      auto const src_addr     = ip::make_address_v4("127.0.0.1");
      auto const src_port     = static_cast<unsigned short>(1337);
      auto const src_endpoint = tcp::endpoint(src_addr, src_port);

      auto const reuse_addr = true;

      auto ctx = foxy::make_ssl_ctx(ssl::context::method::tlsv12_client);
      ctx.load_verify_file("root-cas.pem");

      auto opts = foxy::session_opts{ctx, 30s};

      auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr, opts);
      proxy->async_accept();

      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 30s;

      client.async_connect("127.0.0.1", "1337", yield);

      auto request =
        http::request<http::empty_body>(http::verb::get, "https://www.google.com:443/", 11);

      http::response_parser<http::string_body> res_parser;

      client.async_request(request, res_parser, yield);

      auto response = res_parser.release();

      auto const was_valid_result = (response.result() == http::status::ok);
      auto const was_valid_body =
        (response.body().size() > 0) && boost::string_view(response.body()).ends_with("</html>");

      auto ec = boost::system::error_code();
      client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
      client.stream.plain().close(ec);

      was_valid_response = was_valid_result && was_valid_body;
      proxy->cancel();
    });

    io.run();
    REQUIRE(was_valid_response);
  }
}
