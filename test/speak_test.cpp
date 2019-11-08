//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/speak.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/coroutine.hpp>

#include <boost/beast/http.hpp>

#include <memory>

#include <catch2/catch.hpp>

namespace asio = boost::asio;
namespace http = boost::beast::http;

#include <boost/asio/yield.hpp>

TEST_CASE("speak_test")
{
  SECTION("The speak should function as a basic HTTP client")
  {
    asio::io_context io{1};

    auto const* const host    = "www.google.com";
    auto const* const service = "http";

    auto executor = io.get_executor();

    foxy::speak(executor, host, service, [](auto& client_session) {
      auto req_ = std::make_unique<http::request<http::empty_body>>(http::verb::get, "/", 11);
      req_->set(http::field::host, "www.google.com");

      auto res_ = std::make_unique<http::response<http::string_body>>();

      return [&client_session, coro = asio::coroutine(), req = std::move(req_),
              res = std::move(res_)](auto& self, boost::system::error_code ec = {},
                                     std::size_t bytes_transferred = 0) mutable {
        reenter(coro)
        {
          yield client_session.async_request(*req, *res, std::move(self));
          CHECK(res->result() == http::status::ok);
          CHECK(res->body().size() > 0);

          return self.complete({}, 0);
        }
      };
    });

    io.run();
  }
}
