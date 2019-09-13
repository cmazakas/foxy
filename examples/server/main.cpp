#include <foxy/client_session.hpp>
#include <foxy/server_session.hpp>

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/system/error_code.hpp>

#include <boost/beast/http.hpp>

#include <memory>
#include <iostream>

namespace asio = boost::asio;
namespace http = boost::beast::http;

using boost::asio::ip::tcp;

#include <boost/asio/yield.hpp>

struct server_op : asio::coroutine
{
  using executor_type = asio::executor;

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

  executor_type          executor;
  std::unique_ptr<frame> frame_ptr;

  server_op(asio::executor executor_, tcp::socket stream)
    : executor(executor_)
    , frame_ptr(std::make_unique<frame>(std::move(stream),
                                        foxy::session_opts{{}, std::chrono::seconds(30), false}))
  {
  }

  auto operator()(boost::system::error_code ec = {}, std::size_t const bytes_transferred = 0)
    -> void
  {
    auto& f = *frame_ptr;
    reenter(*this)
    {
      while (true) {
        f.response = {};
        f.request  = {};

        yield f.server.async_read(f.request, std::move(*this));
        if (ec) {
          std::cout << "Encountered error when reading in the request!\n" << ec << "\n";
          goto shutdown;
        }

        std::cout << "Received message!\n" << f.request << "\n";

        f.response.result(http::status::ok);
        f.response.version(11);
        f.response.body() = "<html><p>Hello, world!</p></html>";
        f.response.prepare_payload();

        yield f.server.async_write(f.response, std::move(*this));

        if (ec) {
          std::cout << "Encountered error when writing the response!\n" << ec << "\n";
          goto shutdown;
        }

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
    return executor;
  }
};

struct accept_op : asio::coroutine
{
  using executor_type = asio::executor;

  struct frame
  {
    tcp::socket socket;

    frame(asio::executor executor)
      : socket(executor)
    {
    }
  };

  executor_type          executor;
  std::unique_ptr<frame> frame_ptr;
  tcp::acceptor&         acceptor;

  accept_op(tcp::acceptor& acceptor_)
    : executor(acceptor_.get_executor())
    , frame_ptr(std::make_unique<frame>(executor))
    , acceptor(acceptor_)
  {
  }

  auto operator()(boost::system::error_code ec = {}) -> void
  {
    auto& f = *frame_ptr;
    reenter(*this)
    {
      while (true) {
        yield acceptor.async_accept(f.socket, std::move(*this));
        if (ec == asio::error::operation_aborted) { yield break; }
        if (ec) {
          std::cout << "Failed to accept the new connection!\n";
          std::cout << ec << "\n";
          yield break;
        }

        asio::post(server_op(executor, std::move(f.socket)));
      }
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return executor;
  }
};

struct server
{
  using executor_type = asio::executor;

  executor_type executor;
  tcp::acceptor acceptor;
  tcp::socket   socket;

  server()              = delete;
  server(server const&) = delete;
  server(server&&)      = default;

  server(executor_type executor_, tcp::endpoint endpoint)
    : executor(executor_)
    , acceptor(executor, endpoint)
    , socket(executor)
  {
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return executor;
  }

  auto
  async_accept() -> void
  {
    asio::post(accept_op(acceptor));
  }

  auto
  shutdown() -> void
  {
    asio::post(executor, [self = this]() mutable -> void {
      self->acceptor.cancel();
      self->acceptor.close();
    });
  }
};

#include <boost/asio/unyield.hpp>

int
main()
{
  asio::io_context io{1};

  auto const endpoint =
    tcp::endpoint(asio::ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337));

  auto s = server(io.get_executor(), endpoint);
  s.async_accept();

  asio::spawn(io.get_executor(), [&](auto yield) mutable -> void {
    auto client = foxy::client_session(io.get_executor(),
                                       foxy::session_opts{{}, std::chrono::seconds(30), false});

    client.async_connect("127.0.0.1", "1337", yield);

    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);
    request.keep_alive(false);

    auto response = http::response<http::string_body>();

    client.async_request(request, response, yield);

    std::cout << "Got response back from server!\n" << response << "\n";

    client.stream.plain().shutdown(tcp::socket::shutdown_both);
    client.stream.plain().close();

    s.shutdown();
  });

  io.run();

  return 0;
}
