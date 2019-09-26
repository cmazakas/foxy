//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

// This example builds upon the first HTTP server example that was single-threaded.
// We now introduce TLS 1.2 using a locally signed server certificate for the IP address: 127.0.0.1
// This enables the example to use domain name verification without requiring users to also update
// their hosts file
//

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

// Users can configure this to play around with stress-testing the example server
// It is recommended that users doing this also build in Release mode as well
// It can quite easy to overwhelm the acceptor.
//
static constexpr int num_client_requests = 10;
static constexpr int num_clients         = 128;

#include <boost/asio/yield.hpp>

struct server_op : asio::coroutine
{
  // this time, our executor_type is a strand over the polymorphic executor
  //
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

  // Note, we pass an `ssl::context&` this time and also make sure that our strand is created around
  // the server session's executor
  // We do this because we want synchronicity for all intermediate operations associated with this
  // server session explicitly
  //
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

      // start the server-side handshake process
      // if we fail here, simply just close the underlying socket like we would a plain TCP
      // connection
      //
      yield f.server.async_handshake(std::move(*this));
      if (ec) {
        f.server.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
        f.server.stream.plain().close(ec);
        yield break;
      }

      // now begin the persistent connection read-loop
      //
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

      // gracefully close the connection with the client
      //
      yield f.server.stream.ssl().async_shutdown(std::move(*this));
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

  // This time around, our acceptance operation stores a non-owning reference to an SSL context.
  // It also now stores a strand as well for thread-safety
  //
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
        if (ec) { yield break; }

        // Forward the SSL context to the server operation
        //
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
  // Our server now also stores a non-owning reference to an SSL context so that it may forward
  // it to the accept op which forwards it to the server op
  //
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
    http::request<http::empty_body>   request;
    http::response<http::string_body> response;

    int       req_count    = 0;
    int const max_requests = num_client_requests;

    // Note: this time around, we're constructing our client with verification of the peer
    // certificate enabled. Because our session options also contain an SSL context, this will
    // force the client to initiate the TLS handshake upon `async_connect` and also verify the
    // validity and identity of the server's cert.
    //
    frame(asio::executor executor, ssl::context& ctx)
      : client(executor, foxy::session_opts{ctx, std::chrono::seconds(30), true})
    {
    }
  };

  // This time, we use an atomic `req_count` variable to keep track of all the client requests.
  // When we've reached our threshold, we send the cancellation signal to the server
  //
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

      yield f.client.async_connect("127.0.0.1", "1337", std::move(*this));
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
          if (curr_req_count == num_clients * num_client_requests) { s.shutdown(); }
        }
      }

      // Gracefully close the connection down
      //
      yield f.client.stream.ssl().async_shutdown(std::move(*this));
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
};

#include <boost/asio/unyield.hpp>

// This function is largely a copy-paste of the `load_server_ceritificate` function found in the
// Beast examples. We, however, choose to use a different certificate. We use a locally-signed
// certificate for the IP address 127.0.0.1 and we also skip touching anything to do with the DH
// params involved in the handshake process.
//
auto
load_server_certificate(boost::asio::ssl::context& ctx) -> void
{
  auto const cert = boost::string_view(
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDWzCCAkMCFEkgH0LSdOVvLn/i+NRjXLmhgXjGMA0GCSqGSIb3DQEBCwUAMHsx\n"
    "CzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9ybmlhMRMwEQYDVQQHDApTYWNy\n"
    "YW1lbnRvMSAwHgYDVQQKDBdDb2RlciBpbiBhIENhdmUgU3R1ZGlvczEgMB4GA1UE\n"
    "AwwXQ29kZXIgaW4gYSBDYXZlIFN0dWRpb3MwHhcNMTkwOTE1MjM0MjQzWhcNMjQw\n"
    "OTEzMjM0MjQzWjBZMQswCQYDVQQGEwJVUzETMBEGA1UECAwKU29tZS1TdGF0ZTEh\n"
    "MB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRIwEAYDVQQDDAkxMjcu\n"
    "MC4wLjEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDhXSGTS78ZuAJN\n"
    "ou764hvCOP9yY02sgCcZJ0Ykni8SK/cE0USCe1VEiWgwcedLAJgRvkEhtvinsSFk\n"
    "mUGkmaOy+dwg0lgjmuIlhsi3rA2ngprjcLVqPriwNfVBhkKb+cztwjnrNomxq/Ab\n"
    "/sX2lDAb6VbfVTNnmMT5QbOljzYf82gWc6pYcm5pixooQBD+W52ehq/jpPMYvoqY\n"
    "Vqnz/XKT8NvO3bMhbG2hgKxlocpznog61ih9N39OISbdLDbNXNrkpFfs/ST3I2mK\n"
    "5juVyQuBoJftJOHc+xtlPRcJfRMWSigGrXc4Nv61GJgBv+PwqpJpwjHe/bEc2h8D\n"
    "JJpZ1tuZAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAJWIJoWb035YgXRrW8Qd0TWy\n"
    "n3DmFdN0AEaS7JVrA8zqpGNwZmShvLV4YEEasO/NHXU0qpxuoiKhAAB+e/QEutBP\n"
    "o5gopns4T3irRJYb++RDaVbJwpl1VmenSRDkg5wQT6ZoH3l4coxrD8nbF2mOz3jP\n"
    "VGuG0JhT80rGuOyHVgRz2ilRMgsBc0svPcIo9oNHFQnDCp4uArBhRp+LEK72nZ3J\n"
    "X75SCLwET3fg5QCsfvGQp9cvBqgMWYL9NhXD68xkLlzoWVoGZeM4Pe/QS9A/Eyqu\n"
    "m48CeYe3QQpC395iTjgn0eW6Y/Qaicu7znwIfaVPS+049Lp/HdKL4jFjiv2Kt/A=\n"
    "-----END CERTIFICATE-----\n");

  auto const key = boost::string_view(
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEpQIBAAKCAQEA4V0hk0u/GbgCTaLu+uIbwjj/cmNNrIAnGSdGJJ4vEiv3BNFE\n"
    "gntVRIloMHHnSwCYEb5BIbb4p7EhZJlBpJmjsvncINJYI5riJYbIt6wNp4Ka43C1\n"
    "aj64sDX1QYZCm/nM7cI56zaJsavwG/7F9pQwG+lW31UzZ5jE+UGzpY82H/NoFnOq\n"
    "WHJuaYsaKEAQ/ludnoav46TzGL6KmFap8/1yk/Dbzt2zIWxtoYCsZaHKc56IOtYo\n"
    "fTd/TiEm3Sw2zVza5KRX7P0k9yNpiuY7lckLgaCX7STh3PsbZT0XCX0TFkooBq13\n"
    "ODb+tRiYAb/j8KqSacIx3v2xHNofAySaWdbbmQIDAQABAoIBAQDJ7AgJUfUHtjda\n"
    "GILHh5AXlbpLY25VAP4HK4lNhe3m+j15s4cO4jKkFfmkbmouaXnXbAAvlSF2Ht8s\n"
    "o6SNNpvV4Mm7HryaKkw2E24EI8SYMg1Ve8cQSuJv/+ifrQxBdLCI113Nwi/dYZDh\n"
    "hIUbSetRFuEfedd1GwxhyNyNmqOEO4R6dnt1rTmFhtXqHaUWbrCuTp5t3JrnQQO+\n"
    "jPgLd35G9A1hjXQ0CFUzkd1JuVJdx/U6nN4JKWc2qimQ8t1YpPQ0v8I6Q2DrPmIe\n"
    "pY7bh7wheZ2JwsJ0spgtTfUwCP3LZfQ/UW5BdjHtxnwlyuoHWLB8dgD8XfJHvwd4\n"
    "+p1HN7YBAoGBAPiIMOUk3rU2hOklLU3TvpD4/GtzhO08KR9T4XrPG4lMJ2/fx4OG\n"
    "XqYFWA1hkY7f1Lc9L1LxeOUAfNEagC7K9nz8ESVa8ink4X2F1IwIyDAU19aTvYK4\n"
    "9e4LrM9C9td6BQF0r3jemmN1qjV1naRWj4z8MlW3O2hBRo9RijOVMrQ1AoGBAOgi\n"
    "uI9v8mJXtrDeTAjVbSqnfw0Ra3WXi1ONfOXvPjldnCHCmwsv7PRpET/jvPZrWvaB\n"
    "96RpiEMcClDCaJX62zhEtCk/2Or2xvSureCnRavzmFmKF2Ehjom0ova6rImlmdWh\n"
    "u2NpfxXb/nR9mBClrElG47diW4qucHpiNo4aEa5VAoGAdAjx+yoZqLWJnGi1HC8O\n"
    "PBVjlK9ckn6SHIRHM9VaX+HkT8FFH00vB4hbMfQpx3ENmXfBjpIbBaASpnYe/rnY\n"
    "F0aAotYxVgn8lWRUdgTrojc5Bn/37P56I+fjiOkU4kmf6KwX+PDFWEZpb4g4T6/y\n"
    "WbqtrYNdAzHmxacmRSsVfzkCgYEA3cAVSEhrZdBen9SrE6E1+JIqx0QFwD51BOrb\n"
    "Dhed/FTVClcJnwU4OT6JENwvrcJeEa+T7oY1ec42eHFOUT9i3Pycke8A+2ukIScg\n"
    "yMNhxeIcfiRxMwNIU3mwVzt6CL+eFbq69DtaAHq4N3WmpvhsfU9vxsX5pp/+qJpb\n"
    "fSGgFEUCgYEA6iyrZxQ5QDIZMqzJYXsB6FW4TnAB5sEUTWzoOfsvs3iRMB/y/yhp\n"
    "pG1N/bFkTnAmwU8/Hp9ZozFXMUxb3AHnWMeg6YPhTYvUj5X1NzsCvSR3KX+V/crC\n"
    "84+kQvgny42OaKjwmvTbRCqv7/iOZzSqCSwMytcFVx11NO4+FMFSUp4=\n"
    "-----END RSA PRIVATE KEY-----\n");

  ctx.set_options(boost::asio::ssl::context::default_workarounds |
                  boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::single_dh_use);

  ctx.use_certificate_chain(boost::asio::buffer(cert.data(), cert.size()));

  ctx.use_private_key(boost::asio::buffer(key.data(), key.size()),
                      boost::asio::ssl::context::file_format::pem);
}

int
main()
{
  // Create an SSL context specifically to be used by our server
  //
  auto server_ctx = ssl::context(ssl::context::method::tlsv12_server);
  load_server_certificate(server_ctx);

  // This is our root CA that was used to sign the server's certificate
  //
  auto const root_ca = boost::string_view(
    "-----BEGIN CERTIFICATE-----\n"
    "MIID1zCCAr+gAwIBAgIUBzm5/pOjqdVMVfoTQgGf8RxiuAkwDQYJKoZIhvcNAQEL\n"
    "BQAwezELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExEzARBgNVBAcM\n"
    "ClNhY3JhbWVudG8xIDAeBgNVBAoMF0NvZGVyIGluIGEgQ2F2ZSBTdHVkaW9zMSAw\n"
    "HgYDVQQDDBdDb2RlciBpbiBhIENhdmUgU3R1ZGlvczAeFw0xOTA5MTUyMzI4MDha\n"
    "Fw0yNDA5MTMyMzI4MDhaMHsxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9y\n"
    "bmlhMRMwEQYDVQQHDApTYWNyYW1lbnRvMSAwHgYDVQQKDBdDb2RlciBpbiBhIENh\n"
    "dmUgU3R1ZGlvczEgMB4GA1UEAwwXQ29kZXIgaW4gYSBDYXZlIFN0dWRpb3MwggEi\n"
    "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDDBUCUPopmLtS8Zpd9zMz0+9Br\n"
    "iP4DqRfciqe9ltRBJGvFVaXpyDjknQPcl+cfKiw/4DKRDUnb6EmGk3v8PcVtPIF7\n"
    "ko59URmMelgxdhQSW6oraUGp0xE9UVsRVJYNpnnqAg0rUwwTKAKJx+7aAx75eWEn\n"
    "seWFZK7plZwGFDkvEvAgKsXDSZGjTqgcsbpWaJQ1o3XRhNcIfakhe6pA4hVlIBqM\n"
    "NgeDqCwLgvnRwqNtfFPNCcjizIngX7Jc48MhtgcGKKYo1ddUwE9lj32kB5rpk5wt\n"
    "a6fPkvWYDFu2gWDgtxxEO0bJJkFxPRbppNHxQp00h8ojk+QFDePCd9EAGGPDAgMB\n"
    "AAGjUzBRMB0GA1UdDgQWBBQIo4VHujILmom1rPijIu1U8ndLZDAfBgNVHSMEGDAW\n"
    "gBQIo4VHujILmom1rPijIu1U8ndLZDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3\n"
    "DQEBCwUAA4IBAQBcnGgLMMRhLq0WuoedkxG/G6zuStoDiuGtKzg0TYvdgegOjpHl\n"
    "PsCQmCEUcPMFoo+ohqlZMwoLTZNv02JpRaiBGbOFFNvLQM/cyQ5neGb+o5zmoJAW\n"
    "jG5gzY1mQB3KGq+tP/IBvvdGkq7aqC3Qmnvqr5+3qO6sFzQlXGuH6Qwai8mZJhLB\n"
    "zV6lehqLWuFiWfmAe08V9jDSaMR4mQEYPdi4vkxu8L1/yI12tptdkYoTleQ+qJiy\n"
    "CGwuI6o0hcJWpOEJBkut35FagHqjL54ORLUYQ13kRIaHQkSQ8UgAEa/TbhGDqqjU\n"
    "bPbxZffpGVFTPP02ILYH3/cS6hN1vjltNk/4\n"
    "-----END CERTIFICATE-----\n");

  auto client_ctx = foxy::make_ssl_ctx(ssl::context::method::tlsv12_client);
  client_ctx.add_certificate_authority(asio::const_buffer(root_ca.data(), root_ca.size()));

  auto const num_client_threads = 4;
  auto const num_server_threads = 4;

  // For the sake of performance, we give our client and server separate threadpools
  //
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
