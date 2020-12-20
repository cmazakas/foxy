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
#include <foxy/server_session.hpp>
#include <foxy/utility.hpp>

#include <foxy/test/helpers/ssl_ctx.hpp>

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

static constexpr int num_client_requests = 10;
static constexpr int num_clients         = 128;

#include <boost/asio/yield.hpp>

struct request_handler : asio::coroutine
{
  struct frame
  {
    http::request<http::empty_body>   request;
    http::response<http::string_body> response;
  };

  foxy::server_session&  server;
  std::unique_ptr<frame> frame_ptr;

  request_handler(foxy::server_session& server_)
    : server(server_)
    , frame_ptr(std::make_unique<frame>())
  {
  }

  template <class Self>
  auto operator()(Self&                     self,
                  boost::system::error_code ec                = {},
                  std::size_t const         bytes_transferred = 0) -> void
  {
    assert(server.stream.is_ssl());

    auto& f = *frame_ptr;
    reenter(*this)
    {
      while (true) {
        f.response = {};
        f.request  = {};

        yield server.async_read(f.request, std::move(self));
        if (ec) { break; }

        f.response.result(http::status::ok);
        f.response.version(11);
        f.response.body() = "<html><p>Hello, world!</p></html>";
        f.response.prepare_payload();

        yield server.async_write(f.response, std::move(self));
        if (ec) { break; }

        if (!f.request.keep_alive()) { break; }
      }

      return self.complete({}, 0);
    }
  }
};

struct client_op : asio::coroutine
{
  using executor_type = asio::strand<asio::any_io_executor>;

  struct frame
  {
    foxy::client_session              client;
    http::request<http::empty_body>   request;
    http::response<http::string_body> response;

    int       req_count    = 0;
    int const max_requests = num_client_requests;

    frame(asio::any_io_executor executor, ssl::context& ctx)
      : client(executor, foxy::session_opts{ctx, std::chrono::seconds(30), true})
    {
    }
  };

  std::unique_ptr<frame> frame_ptr;
  executor_type          strand;
  std::atomic_int&       req_count;
  foxy::listener&        l;

  client_op(asio::any_io_executor   executor,
            std::atomic_int& req_count_,
            foxy::listener&  l_,
            ssl::context&    ctx)
    : frame_ptr(std::make_unique<frame>(executor, ctx))
    , strand(asio::make_strand(executor))
    , req_count(req_count_)
    , l(l_)
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
          if (curr_req_count == num_clients * num_client_requests) { l.shutdown(); }
        }
      }

      yield f.client.async_shutdown(std::move(*this));
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
};

int
main()
{
  auto server_ctx = foxy::test::make_server_ssl_ctx();
  auto client_ctx = foxy::test::make_client_ssl_ctx();

  auto const num_client_threads = 4;
  auto const num_server_threads = 4;

  // For the sake of performance, we give our client and server separate threadpools
  //
  asio::io_context client_io{num_client_threads};
  asio::io_context server_io{num_server_threads};

  std::atomic_int req_count{0};

  auto s = foxy::listener(
    server_io.get_executor(),
    tcp::endpoint(asio::ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337)),
    std::move(server_ctx));

  s.async_accept([](auto& server_session) { return request_handler(server_session); });

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
