//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/detail/tunnel.hpp>
#include <foxy/session.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/experimental/test/stream.hpp>

#include <catch2/catch.hpp>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;

using stream_type = beast::test::stream;

TEST_CASE("Our async tunnel operation")
{
  SECTION("should enable absolute URIs as a valid test target")
  {
    asio::io_context io;

    auto req_stream = stream_type(io);
    auto res_stream = stream_type(io);

    auto server_stream = stream_type(io);
    auto client_stream = stream_type(io);

    auto server = foxy::basic_session<stream_type>(std::move(server_stream));
    auto client = foxy::basic_session<stream_type>(std::move(client_stream));

    auto request =
      http::request<http::empty_body>(http::verb::get, "http://www.some-server.com/path.html", 11);

    beast::ostream(server.stream.plain().buffer()) << request;
  }
}
