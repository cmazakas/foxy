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
#include <foxy/utility.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include <boost/beast/http.hpp>

#include <vector>
#include <memory>
#include <thread>
#include <string>
#include <atomic>

#include <catch2/catch.hpp>

using boost::asio::ip::tcp;
namespace ip   = boost::asio::ip;
namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace ssl  = boost::asio::ssl;

using namespace std::chrono_literals;

TEST_CASE("proxy_test2")
{
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
      client.opts.timeout = 5s;

      client.async_connect("127.0.0.1", "1337", yield);

      auto request = http::request<http::empty_body>(http::verb::connect, "www.google.com", 11);
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

      auto opts = foxy::session_opts{ctx, 5s};

      auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr, opts);
      proxy->async_accept();

      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 5s;

      client.async_connect("127.0.0.1", "1337", yield);

      auto request = http::request<http::empty_body>(http::verb::get, "https://www.google.com", 11);

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

  SECTION("should forward a one-time relay with query params")
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

      auto request = http::request<http::empty_body>(
        http::verb::get, "https://www.bing.com/search?q=boostorg+beast", 11);

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

  SECTION("should work in multithreaded environments")
  {
    auto const urls =
      std::vector<std::string>{"www.google.com", "www.facebook.com",  "www.youtube.com",
                               "www.yahoo.com",  "www.wikipedia.org", "www.twitter.com",
                               "www.amazon.com", "www.google.com",    "www.bing.com"};

    auto const num_threads = 8;

    asio::io_context io(num_threads);

    auto const src_addr     = ip::make_address_v4("127.0.0.1");
    auto const src_port     = static_cast<unsigned short>(1337);
    auto const src_endpoint = tcp::endpoint(src_addr, src_port);

    auto const reuse_addr = true;

    auto const num_requests = urls.size() * num_threads;

    std::atomic_int num_valid_responses{0};

    auto ctx = foxy::make_ssl_ctx(ssl::context::method::tlsv12_client);
    ctx.load_verify_file("root-cas.pem");

    auto opts = foxy::session_opts{ctx, 30s};

    auto proxy = std::make_shared<foxy::proxy>(io, src_endpoint, reuse_addr, opts);
    proxy->async_accept();

    auto const crawler = [&io, &urls, &num_valid_responses, proxy,
                          num_requests](asio::yield_context yield) -> void {
      auto client         = foxy::client_session(io.get_executor(), {});
      client.opts.timeout = 30s;

      for (auto const url : urls) {
        client.async_connect("127.0.0.1", "1337", yield);

        auto request = http::request<http::empty_body>(http::verb::get, "https://" + url + "/", 11);
        request.set(http::field::host, url);

        http::response_parser<http::string_body> res_parser;
        client.async_request(request, res_parser, yield);

        auto response = res_parser.release();

        auto const was_valid_result = (response.result() != http::status::internal_server_error &&
                                       response.result() != http::status::not_found);

        auto ec = boost::system::error_code();
        client.stream.plain().shutdown(tcp::socket::shutdown_send, ec);
        client.stream.plain().close(ec);

        if (was_valid_result) {
          auto const num_ops = ++num_valid_responses;
          if (num_ops == num_requests) { proxy->cancel(); }
        }
      }
    };

    for (auto idx = 0; idx < num_threads; ++idx) { asio::spawn(io.get_executor(), crawler); }

    auto threads = std::vector<std::thread>();
    threads.reserve(num_threads);

    for (auto idx = 0; idx < num_threads; ++idx) {
      threads.emplace_back([&io] { io.run(); });
    }

    for (auto& t : threads) { t.join(); }

    REQUIRE(num_valid_responses == num_requests);
  }
}
