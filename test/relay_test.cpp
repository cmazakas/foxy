//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#include <foxy/detail/relay.hpp>
#include <foxy/session.hpp>

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

    auto req_stream    = stream_type(io);
    auto res_stream    = stream_type(io);
    auto server_stream = stream_type(io);
    auto client_stream = stream_type(io);

    auto server = foxy::basic_session<stream_type>(std::move(server_stream));
    auto client = foxy::basic_session<stream_type>(std::move(client_stream));

    server.opts.timeout = std::chrono::seconds{5};
    client.opts.timeout = std::chrono::seconds{5};

    req_stream.connect(server.stream.plain());
    client.stream.plain().connect(res_stream);

    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);
    request.prepare_payload();

    beast::ostream(server.stream.plain().buffer()) << request;

    auto response = http::response<http::string_body>(
      http::status::ok, 11,
      "I bestow the heads of virgins and the first-born sons!!!!\n");
    response.prepare_payload();

    beast::ostream(res_stream.buffer()) << response;

    asio::spawn(
      [&](asio::yield_context yield) mutable
      {
        std::cout << "starting relay\n";
        auto const bytes_transferred = foxy::detail::async_relay(server, client, yield);
        std::cout << "ending relay\n";
      });

    io.run();

    CHECK(req_stream.str() == "I bestow the heads of virgins and the first-born sons!!!!\n");
  }
}
