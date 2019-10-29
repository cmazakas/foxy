//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/listener.hpp>
#include <foxy/client_session.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/coroutine.hpp>

#include <boost/beast/http.hpp>

#include <boost/utility/string_view.hpp>

#include <iostream>

#include <foxy/test/helpers/ssl_ctx.hpp>
#include <catch2/catch.hpp>

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace ssl  = boost::asio::ssl;

using boost::asio::ip::tcp;

namespace
{
#include <boost/asio/yield.hpp>
struct handler : asio::coroutine
{
  foxy::server_session& server;

  std::unique_ptr<http::request<http::empty_body>> request_handle =
    std::make_unique<http::request<http::empty_body>>();

  std::unique_ptr<http::response<http::string_body>> response_handle =
    std::make_unique<http::response<http::string_body>>();

  handler(foxy::server_session& server_)
    : server(server_)
  {
  }

  template <class Self>
  auto operator()(Self& self, boost::system::error_code ec = {}, std::size_t bytes_transferred = 0)
    -> void
  {
    auto& request  = *request_handle;
    auto& response = *response_handle;

    reenter(*this)
    {
      yield server.async_read(request, std::move(self));
      if (ec) { return self.complete(ec, bytes_transferred); }

      response.result(200);
      response.body() = "hello, world!";
      response.prepare_payload();

      yield server.async_write(response, std::move(self));
      if (ec) { return self.complete(ec, bytes_transferred); }

      self.complete({}, 0);
    }
  }
};
#include <boost/asio/unyield.hpp>

auto
make_handler(foxy::server_session& server) -> handler
{
  return handler(server);
}

} // namespace

TEST_CASE("listener_test")
{
  SECTION("Our HTTP listener should be able to process a request")
  {
    asio::io_context io{1};

    auto const endpoint =
      tcp::endpoint(asio::ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337));

    auto listener = foxy::listener(io.get_executor(), endpoint);
    listener.async_accept(&make_handler);

    asio::spawn(io.get_executor(), [&](auto yield) mutable {
      auto client = foxy::client_session(io.get_executor(), {{}, std::chrono::seconds(4), false});
      client.async_connect("127.0.0.1", "1337", yield);

      auto req = http::request<http::empty_body>(http::verb::get, "/", 11);
      auto res = http::response<http::string_body>();

      client.async_request(req, res, yield);

      CHECK(res.result_int() == 200);
      CHECK(res.body() == "hello, world!");

      auto ec = boost::system::error_code();
      client.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
      client.stream.plain().close(ec);

      listener.shutdown();
    });

    io.run();
  }

  SECTION("Out HTTPS listener should be able to process a request")
  {
    auto server_ctx = foxy::test::make_server_ssl_ctx();
    auto client_ctx = foxy::test::make_client_ssl_ctx();

    asio::io_context io{1};

    auto const endpoint =
      tcp::endpoint(asio::ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337));

    auto listener = foxy::listener(io.get_executor(), endpoint, std::move(server_ctx));
    listener.async_accept(&make_handler);

    asio::spawn(io.get_executor(), [&](auto yield) mutable {
      auto client =
        foxy::client_session(io.get_executor(), {client_ctx, std::chrono::seconds(4), true});

      client.async_connect("127.0.0.1", "1337", yield);

      auto req = http::request<http::empty_body>(http::verb::get, "/", 11);
      auto res = http::response<http::string_body>();

      client.async_request(req, res, yield);

      CHECK(res.result_int() == 200);
      CHECK(res.body() == "hello, world!");

      client.async_shutdown(yield);
      listener.shutdown();
    });

    io.run();
  }
}
