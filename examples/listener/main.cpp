#include <foxy/listener.hpp>
#include <foxy/client_session.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <boost/beast/http.hpp>

#include <iostream>

namespace asio = boost::asio;
namespace ip   = boost::asio::ip;
namespace http = boost::beast::http;

using tcp = boost::asio::ip::tcp;

#include <boost/asio/yield.hpp>

int
main()
{
  asio::io_context io{1};

  auto listener =
    foxy::listener(io.get_executor(),
                   tcp::endpoint(ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337)));

  listener.async_accept([&listener](auto& server) {
    return
      [&server, coro = asio::coroutine(),
       request  = std::make_unique<http::request<http::empty_body>>(),
       response = std::make_unique<http::response<http::string_body>>()](
        auto& self, boost::system::error_code ec = {}, std::size_t bytes_transferred = 0) mutable {
        reenter(coro)
        {
          yield server.async_read(*request, std::move(self));
          if (ec) { return self.complete(ec, bytes_transferred); }

          response->result(200);
          response->body() = "hello, world!";
          response->prepare_payload();

          yield server.async_write(*response, std::move(self));
          if (ec) { return self.complete(ec, bytes_transferred); }

          self.complete({}, 0);
        }
      };
  });

  asio::spawn(io.get_executor(), [&](auto yield_ctx) mutable {
    auto client = foxy::client_session(io.get_executor(), {{}, std::chrono::seconds(4), false});
    client.async_connect("127.0.0.1", "1337", yield_ctx);

    auto req = http::request<http::empty_body>(http::verb::get, "/", 11);
    auto res = http::response<http::string_body>();

    client.async_request(req, res, yield_ctx);

    std::cout << "Got the response:\n" << res << "\n";

    auto ec = boost::system::error_code();
    client.async_shutdown(yield_ctx[ec]);

    listener.shutdown();
  });

  io.run();

  return 0;
}

#include <boost/asio/unyield.hpp>
