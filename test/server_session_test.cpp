//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/server_session.hpp>
#include <foxy/client_session.hpp>

#include <boost/asio/ssl.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/io_context.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/_experimental/test/stream.hpp>

#include <catch2/catch.hpp>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;
namespace ip    = boost::asio::ip;
namespace ssl   = boost::asio::ssl;

using boost::asio::ip::tcp;

using test_stream = boost::beast::test::stream;

TEST_CASE("server_session_test")
{
  SECTION("Our server session should detect an SSL handshake")
  {
    asio::io_context io{1};

    auto const addr     = ip::make_address_v4("127.0.0.1");
    auto const port     = static_cast<unsigned short>(1337);
    auto const endpoint = tcp::endpoint(addr, port);

    auto const reuse_addr = true;

    auto acceptor = tcp::acceptor(io, endpoint, reuse_addr);
    REQUIRE(acceptor.is_open());

    auto ctx = ssl::context(ssl::context::tlsv12);

    auto opts    = foxy::session_opts{};
    opts.timeout = std::chrono::seconds{1};
    opts.ssl_ctx = ctx;

    auto client = foxy::client_session(io, opts);

    auto detected_ssl     = false;
    auto handshake_failed = false;

    asio::spawn(io, [&](asio::yield_context yield) mutable -> void {
      auto stream = foxy::multi_stream(io);
      acceptor.async_accept(stream.plain(), yield);

      auto server = foxy::server_session(std::move(stream), {});

      detected_ssl = server.async_detect_ssl(yield);

      auto& socket = server.stream.plain();

      socket.shutdown(tcp::socket::shutdown_both);
      socket.close();
    });

    asio::spawn(io, [&](asio::yield_context yield) mutable -> void {
      auto ec = boost::system::error_code();

      client.async_connect("127.0.0.1", "1337", yield[ec]);

      handshake_failed = static_cast<bool>(ec);

      auto& socket = client.stream.ssl().next_layer();

      socket.shutdown(tcp::socket::shutdown_both);
      socket.close();
    });

    io.run();

    CHECK(detected_ssl);
    CHECK(handshake_failed);
  }
}
