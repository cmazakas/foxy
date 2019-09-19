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

using boost::asio::ip::tcp;

static constexpr int num_client_requests = 10;
static constexpr int num_clients         = 1024;

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
      : server(foxy::multi_stream(std::move(socket)), opts)
    {
    }
  };

  std::unique_ptr<frame> frame_ptr;
  executor_type          strand;

  server_op(tcp::socket stream)
    : frame_ptr(std::make_unique<frame>(std::move(stream),
                                        foxy::session_opts{{}, std::chrono::seconds(30), false}))
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
      while (true) {
        f.response = {};
        f.request  = {};

        yield f.server.async_read(f.request, std::move(*this));
        if (ec) { goto shutdown; }

        f.response.result(http::status::ok);
        f.response.version(11);
        f.response.body() = "<html><p>Hello, world!</p></html>";
        f.response.prepare_payload();

        yield f.server.async_write(f.response, std::move(*this));

        if (ec) { goto shutdown; }

        if (f.request.keep_alive()) { continue; }

      shutdown:
        f.server.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
        f.server.stream.plain().close(ec);
        break;
      }
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

  accept_op(tcp::acceptor& acceptor_, executor_type strand_)
    : acceptor(acceptor_)
    , frame_ptr(std::make_unique<frame>(acceptor.get_executor()))
    , strand(strand_)
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

        asio::post(server_op(std::move(f.socket)));
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

public:
  server()              = delete;
  server(server const&) = delete;
  server(server&&)      = default;

  server(asio::executor executor, tcp::endpoint endpoint)
    : acceptor(executor, endpoint)
    , strand(asio::make_strand(executor))
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
    asio::post(accept_op(acceptor, strand));
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

    frame(asio::executor executor)
      : client(executor, foxy::session_opts{{}, std::chrono::seconds(30), false})
    {
    }
  };

  std::unique_ptr<frame> frame_ptr;
  executor_type          strand;
  std::atomic_int&       req_count;
  server&                s;

  client_op(asio::executor executor, std::atomic_int& req_count_, server& s_)
    : frame_ptr(std::make_unique<frame>(executor))
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
      yield f.client.async_connect("127.0.0.1", "1337", std::move(*this));
      if (ec) { goto shutdown; }

      for (f.req_count = 0; f.req_count < f.max_requests; ++f.req_count) {
        f.request  = {};
        f.response = {};

        f.request.method(http::verb::get);
        f.request.target("/");
        f.request.version(11);
        f.request.keep_alive(f.req_count < (f.max_requests - 1));

        yield f.client.async_request(f.request, f.response, std::move(*this));
        if (ec) { goto shutdown; }

        {
          auto const curr_req_count = ++req_count;
          if (curr_req_count == num_clients * num_client_requests) { s.shutdown(); }
        }
      }

    shutdown:
      f.client.stream.plain().shutdown(tcp::socket::shutdown_both, ec);
      f.client.stream.plain().close(ec);
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
};

#include <boost/asio/unyield.hpp>

int
main()
{
  auto const num_client_threads = 4;
  auto const num_server_threads = 4;

  asio::io_context client_io{num_client_threads};
  asio::io_context server_io{num_server_threads};

  std::atomic_int req_count{0};

  auto const endpoint =
    tcp::endpoint(asio::ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337));

  auto s = server(server_io.get_executor(), endpoint);
  s.async_accept();

  for (auto idx = 0; idx < num_clients; ++idx) {
    asio::post(client_op(client_io.get_executor(), req_count, s));
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
