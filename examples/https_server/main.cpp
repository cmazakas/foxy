#include <foxy/client_session.hpp>
#include <foxy/server_session.hpp>
#include <foxy/utility.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address.hpp>

#include <boost/asio/coroutine.hpp>

#include <boost/asio/executor.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/strand.hpp>

#include <boost/system/error_code.hpp>

#include <boost/beast/http.hpp>

#include <thread>
#include <atomic>
#include <memory>
#include <iostream>
#include <cassert>

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace ssl  = boost::asio::ssl;

using boost::asio::ip::tcp;

static constexpr int num_client_requests = 1;
static constexpr int num_clients         = 1;

#include <boost/asio/yield.hpp>

struct server_op : asio::coroutine
{
  using executor_type = asio::strand<asio::executor>;

  struct frame
  {
    foxy::server_session              server;
    http::request<http::empty_body>   request;
    http::response<http::string_body> response;

    frame(tcp::socket socket, foxy::session_opts opts)
      : server(foxy::multi_stream(std::move(socket), *opts.ssl_ctx), opts)
    {
    }
  };

  std::unique_ptr<frame> frame_ptr;
  executor_type          strand;

  server_op(tcp::socket stream, ssl::context& ctx)
    : frame_ptr(std::make_unique<frame>(std::move(stream),
                                        foxy::session_opts{ctx, std::chrono::seconds(30), false}))
    , strand(asio::make_strand(frame_ptr->server.get_executor()))

  {
  }

  auto operator()(boost::system::error_code ec = {}, std::size_t const bytes_transferred = 0)
    -> void
  {
    assert(strand.running_in_this_thread());

    auto& f = *frame_ptr;
    reenter(*this)
    {
      assert(f.server.stream.is_ssl());

      yield f.server.stream.ssl().async_handshake(ssl::stream_base::server, std::move(*this));
      if (ec) {
        std::cout << "look here! " << ec.message() << "\n";

        f.server.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
        f.server.stream.plain().close(ec);
        yield break;
      }

      while (true) {
        f.response = {};
        f.request  = {};

        yield f.server.async_read(f.request, std::move(*this));
        if (ec) { break; }

        f.response.result(http::status::ok);
        f.response.version(11);
        f.response.body() = "<html><p>Hello, world!</p></html>";
        f.response.prepare_payload();

        yield f.server.async_write(f.response, std::move(*this));
        if (ec) { break; }

        if (!f.request.keep_alive()) { break; }
      }

      yield f.server.stream.ssl().async_shutdown(std::move(*this));

      if (ec) { std::cout << "right here ! " << ec.message() << "\n"; }
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
};

struct accept_op : asio::coroutine
{
  using executor_type = asio::strand<asio::executor>;

  struct frame
  {
    tcp::socket socket;

    frame(asio::executor executor)
      : socket(executor)
    {
    }
  };

  tcp::acceptor&         acceptor;
  std::unique_ptr<frame> frame_ptr;
  executor_type          strand;
  ssl::context&          ctx;

  accept_op(tcp::acceptor& acceptor_, executor_type strand_, ssl::context& ctx_)
    : acceptor(acceptor_)
    , frame_ptr(std::make_unique<frame>(acceptor.get_executor()))
    , strand(strand_)
    , ctx(ctx_)
  {
  }

  auto operator()(boost::system::error_code ec = {}) -> void
  {
    assert(strand.running_in_this_thread());

    auto& f = *frame_ptr;
    reenter(*this)
    {
      while (acceptor.is_open()) {
        yield acceptor.async_accept(f.socket, std::move(*this));
        if (ec == asio::error::operation_aborted) { yield break; }
        if (ec) { std::cout << "uh oh! " << ec.message() << "\n"; }
        if (ec) { yield break; }

        asio::post(server_op(std::move(f.socket), ctx));
      }
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
};

struct server
{
public:
  using executor_type = asio::strand<asio::executor>;

private:
  tcp::acceptor acceptor;
  executor_type strand;
  ssl::context& ctx;

public:
  server()              = delete;
  server(server const&) = delete;
  server(server&&)      = default;

  server(asio::executor executor, tcp::endpoint endpoint, ssl::context& ctx_)
    : acceptor(executor, endpoint)
    , strand(asio::make_strand(executor))
    , ctx(ctx_)
  {
  }

  auto
  get_executor() -> executor_type
  {
    return strand;
  }

  auto
  async_accept() -> void
  {
    asio::post(accept_op(acceptor, strand, ctx));
  }

  auto
  shutdown() -> void
  {
    asio::post(get_executor(), [self = this]() mutable -> void {
      auto ec = boost::system::error_code();

      self->acceptor.cancel(ec);
      self->acceptor.close(ec);
    });
  }
};

struct client_op : asio::coroutine
{
  using executor_type = asio::strand<asio::executor>;

  struct frame
  {
    foxy::client_session              client;
    http::request<http::empty_body>   request = {http::verb::get, "/", 11};
    http::response<http::string_body> response;

    int       req_count    = 0;
    int const max_requests = num_client_requests;

    frame(asio::executor executor, ssl::context& ctx)
      : client(executor, foxy::session_opts{ctx, std::chrono::seconds(30), false})
    {
    }
  };

  std::unique_ptr<frame> frame_ptr;
  executor_type          strand;
  std::atomic_int&       req_count;
  server&                s;

  client_op(asio::executor executor, std::atomic_int& req_count_, server& s_, ssl::context& ctx)
    : frame_ptr(std::make_unique<frame>(executor, ctx))
    , strand(asio::make_strand(executor))
    , req_count(req_count_)
    , s(s_)
  {
  }

  auto
  operator()(boost::system::error_code ec, tcp::endpoint) -> void
  {
    (*this)(ec);
  }

  auto operator()(boost::system::error_code ec = {}) -> void
  {
    assert(strand.running_in_this_thread());

    auto& f = *frame_ptr;
    reenter(*this)
    {
      assert(f.client.stream.is_ssl());

      yield f.client.async_connect("www.example.com", "1337", std::move(*this));
      if (ec) { break; }

      for (f.req_count = 0; f.req_count < f.max_requests; ++f.req_count) {
        f.request  = {};
        f.response = {};

        f.request.method(http::verb::get);
        f.request.target("/");
        f.request.version(11);
        f.request.keep_alive(f.req_count < (f.max_requests - 1));

        yield f.client.async_request(f.request, f.response, std::move(*this));
        if (ec) { break; }

        {
          auto const curr_req_count = ++req_count;
          std::cout << "curr req count " << curr_req_count << "\n";
          if (curr_req_count == num_clients * num_client_requests) { s.shutdown(); }
        }
      }

      if (ec) { std::cout << "error! " << ec.message() << "\n"; }

      yield f.client.stream.ssl().async_shutdown(std::move(*this));
      // f.client.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
      // f.client.stream.plain().close(ec);
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
};

#include <boost/asio/unyield.hpp>

// self-signging certs is lame so copy-paste everything directly from the Beast examples
//

//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

/*  Load a signed certificate into the ssl context, and configure
    the context for use with a server.

    For this to work with the browser or operating system, it is
    necessary to import the "Beast Test CA" certificate into
    the local certificate store, browser, or operating system
    depending on your environment Please see the documentation
    accompanying the Beast certificate for more details.
*/
inline void
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

int
main()
{
  auto server_ctx = ssl::context(ssl::context::method::tlsv12_server);
  load_server_certificate(server_ctx);

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

  auto client_ctx = foxy::make_ssl_ctx(ssl::context::method::tlsv12_client);
  client_ctx.add_certificate_authority(asio::const_buffer(cert.data(), cert.size()));

  auto const num_client_threads = 4;
  auto const num_server_threads = 4;

  asio::io_context client_io{num_client_threads};
  asio::io_context server_io{num_server_threads};

  std::atomic_int req_count{0};

  auto const endpoint =
    tcp::endpoint(asio::ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337));

  auto s = server(server_io.get_executor(), endpoint, server_ctx);
  s.async_accept();

  for (auto idx = 0; idx < num_clients; ++idx) {
    asio::post(client_op(client_io.get_executor(), req_count, s, client_ctx));
  }

  auto threads = std::vector<std::thread>();
  threads.reserve(num_client_threads + num_server_threads);

  for (auto idx = 0; idx < num_client_threads; ++idx) {
    threads.emplace_back([&] { client_io.run(); });
  }

  for (auto idx = num_client_threads; idx < (num_client_threads + num_server_threads); ++idx) {
    threads.emplace_back([&] { server_io.run(); });
  }

  for (auto& thread : threads) { thread.join(); }

  return 0;
}
