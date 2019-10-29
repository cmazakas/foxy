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

#include <catch2/catch.hpp>

#include <iostream>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;
namespace ip    = boost::asio::ip;
namespace ssl   = boost::asio::ssl;

using boost::asio::ip::tcp;

// We shamelessly rip the load server context function from Beast's examples
//
namespace
{
//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

void
load_server_certificate(boost::asio::ssl::context& ctx)
{
  /*
      The certificate was generated from CMD.EXE on Windows 10 using:

      winpty openssl dhparam -out dh.pem 2048
      winpty openssl req -newkey rsa:2048 -nodes -keyout key.pem -x509 -days 10000 -out cert.pem
     -subj "//C=US\ST=CA\L=Los Angeles\O=Beast\CN=www.example.com"
  */

  std::string const cert =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
    "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
    "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
    "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
    "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
    "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
    "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
    "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
    "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
    "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
    "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
    "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
    "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
    "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
    "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
    "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
    "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
    "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
    "UEVbkhd5qstF6qWK\n"
    "-----END CERTIFICATE-----\n";

  std::string const key =
    "-----BEGIN PRIVATE KEY-----\n"
    "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
    "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
    "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
    "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
    "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
    "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
    "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
    "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
    "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
    "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
    "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
    "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
    "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
    "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
    "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
    "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
    "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
    "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
    "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
    "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
    "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
    "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
    "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
    "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
    "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
    "pVOUFq5mW8p0zbtfHbjkgxyF\n"
    "-----END PRIVATE KEY-----\n";

  std::string const dh =
    "-----BEGIN DH PARAMETERS-----\n"
    "MIIBCAKCAQEArzQc5mpm0Fs8yahDeySj31JZlwEphUdZ9StM2D8+Fo7TMduGtSi+\n"
    "/HRWVwHcTFAgrxVdm+dl474mOUqqaz4MpzIb6+6OVfWHbQJmXPepZKyu4LgUPvY/\n"
    "4q3/iDMjIS0fLOu/bLuObwU5ccZmDgfhmz1GanRlTQOiYRty3FiOATWZBRh6uv4u\n"
    "tff4A9Bm3V9tLx9S6djq31w31Gl7OQhryodW28kc16t9TvO1BzcV3HjRPwpe701X\n"
    "oEEZdnZWANkkpR/m/pfgdmGPU66S2sXMHgsliViQWpDCYeehrvFRHEdR9NV+XJfC\n"
    "QMUk26jPTIVTLfXmmwU0u8vUkpR7LQKkwwIBAg==\n"
    "-----END DH PARAMETERS-----\n";

  ctx.set_password_callback(
    [](std::size_t, boost::asio::ssl::context_base::password_purpose) { return "test"; });

  ctx.set_options(boost::asio::ssl::context::default_workarounds |
                  boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);

  ctx.use_certificate_chain(boost::asio::buffer(cert.data(), cert.size()));

  ctx.use_private_key(boost::asio::buffer(key.data(), key.size()),
                      boost::asio::ssl::context::file_format::pem);

  ctx.use_tmp_dh(boost::asio::buffer(dh.data(), dh.size()));
}
} // namespace

TEST_CASE("server_session_test")
{
  SECTION("Our server session should detect an SSL handshake")
  {
    asio::io_context io{1};

    auto const addr     = ip::make_address_v4("127.0.0.1");
    auto const port     = static_cast<unsigned short>(1337);
    auto const endpoint = tcp::endpoint(addr, port);

    auto const reuse_addr = true;

    auto acceptor = tcp::acceptor(io.get_executor(), endpoint, reuse_addr);
    REQUIRE(acceptor.is_open());

    auto ctx = ssl::context(ssl::context::tlsv12);

    auto opts    = foxy::session_opts{};
    opts.timeout = std::chrono::seconds{1};
    opts.ssl_ctx = ctx;

    auto client = foxy::client_session(io.get_executor(), opts);

    asio::spawn(io, [&](asio::yield_context yield) mutable -> void {
      auto ec = boost::system::error_code();

      auto stream = foxy::multi_stream(io.get_executor());
      acceptor.async_accept(stream.plain(), yield);

      auto server = foxy::server_session(std::move(stream), {});

      auto const detected_ssl = server.async_detect_ssl(yield);
      CHECK(detected_ssl);

      auto& socket = server.stream.plain();

      socket.shutdown(tcp::socket::shutdown_both, ec);
      socket.close(ec);
    });

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) mutable -> void {
      auto ec = boost::system::error_code();

      client.async_connect("127.0.0.1", "1337", yield[ec]);

      auto const handshake_failed = static_cast<bool>(ec);
      CHECK(handshake_failed);

      auto& socket = client.stream.ssl().next_layer();

      socket.shutdown(tcp::socket::shutdown_both, ec);
      socket.close(ec);
    });

    io.run();
  }

  SECTION("Our server session should be upgrade-able to TLS upon detection")
  {
    asio::io_context io{1};

    auto const addr     = ip::make_address_v4("127.0.0.1");
    auto const port     = static_cast<unsigned short>(1337);
    auto const endpoint = tcp::endpoint(addr, port);

    auto const reuse_addr = true;

    auto acceptor = tcp::acceptor(io.get_executor(), endpoint, reuse_addr);
    REQUIRE(acceptor.is_open());

    auto client_ctx = ssl::context(ssl::context::tlsv12_client);
    auto server_ctx = ssl::context(ssl::context::tlsv12_server);

    load_server_certificate(server_ctx);

    auto opts    = foxy::session_opts{};
    opts.timeout = std::chrono::seconds{30};
    opts.ssl_ctx = client_ctx;

    auto server_opts = foxy::session_opts{server_ctx, std::chrono::seconds{30}};

    auto client = foxy::client_session(io.get_executor(), opts);
    client.stream.ssl().set_verify_mode(ssl::verify_none);

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) mutable -> void {
      auto ec = boost::system::error_code();

      auto stream = foxy::multi_stream(io.get_executor());
      acceptor.async_accept(stream.plain(), yield);

      auto server = foxy::server_session(std::move(stream), server_opts);
      REQUIRE_FALSE(server.stream.is_ssl());

      auto const detected_ssl = server.async_detect_ssl(yield);
      REQUIRE(detected_ssl);

      auto const bytes_used = server.async_handshake(yield[ec]);

      CHECK(server.buffer.size() == 0);
      CHECK(bytes_used > 0);

      REQUIRE_FALSE(ec);

      server.stream.ssl().async_shutdown(yield);
    });

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) mutable -> void {
      auto ec = boost::system::error_code();

      client.async_connect("127.0.0.1", "1337", yield[ec]);

      auto const handshake_failed = static_cast<bool>(ec);
      CHECK_FALSE(handshake_failed);

      client.async_shutdown(yield);
    });

    io.run();
  }

  SECTION("Our server session should be upgrade-able to TLS (no detection)")
  {
    asio::io_context io{1};

    auto const addr     = ip::make_address_v4("127.0.0.1");
    auto const port     = static_cast<unsigned short>(1337);
    auto const endpoint = tcp::endpoint(addr, port);

    auto const reuse_addr = true;

    auto acceptor = tcp::acceptor(io.get_executor(), endpoint, reuse_addr);
    REQUIRE(acceptor.is_open());

    auto client_ctx = ssl::context(ssl::context::tlsv12_client);
    auto server_ctx = ssl::context(ssl::context::tlsv12_server);

    load_server_certificate(server_ctx);

    auto opts    = foxy::session_opts{};
    opts.timeout = std::chrono::seconds{30};
    opts.ssl_ctx = client_ctx;

    auto server_opts = foxy::session_opts{server_ctx, std::chrono::seconds{30}};

    auto client = foxy::client_session(io.get_executor(), opts);
    client.stream.ssl().set_verify_mode(ssl::verify_none);

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) mutable -> void {
      auto ec = boost::system::error_code();

      auto stream = foxy::multi_stream(io.get_executor());
      acceptor.async_accept(stream.plain(), yield);

      auto server = foxy::server_session(std::move(stream), server_opts);
      server.stream.upgrade(server_ctx);
      REQUIRE(server.stream.is_ssl());

      server.stream.ssl().set_verify_mode(ssl::verify_none);

      auto const bytes_used = server.async_handshake(yield[ec]);

      CHECK(server.buffer.size() == 0);
      CHECK(bytes_used == 0);

      REQUIRE_FALSE(ec);

      server.stream.ssl().async_shutdown(yield);
    });

    asio::spawn(io.get_executor(), [&](asio::yield_context yield) mutable -> void {
      auto ec = boost::system::error_code();

      client.async_connect("127.0.0.1", "1337", yield[ec]);

      auto const handshake_failed = static_cast<bool>(ec);
      CHECK_FALSE(handshake_failed);

      client.async_shutdown(yield);
    });

    io.run();
  }
}
