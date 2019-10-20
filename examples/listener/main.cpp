#include <foxy/listener.hpp>
#include <foxy/client_session.hpp>
#include <foxy/pct_encode.hpp>
#include <foxy/code_point_iterator.hpp>

// we use this to setup our SSL contexts for the server and client
// these shouldn't really be considered production-quality but for the sake of an example, they'll
// have to do
//
#include <foxy/test/helpers/ssl_ctx.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>

#include <boost/beast/http.hpp>

#include <iostream>
#include <memory>
#include <string>

#include <boost/asio/yield.hpp>

namespace asio = boost::asio;
namespace ip   = boost::asio::ip;
namespace http = boost::beast::http;

using tcp = boost::asio::ip::tcp;

int
main()
{
  asio::io_context io{1};

  auto listener =
    foxy::listener(io.get_executor(),
                   tcp::endpoint(ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337)));

  listener.async_accept([&listener, &io](auto& server) {
    static auto client_ssl_ctx = foxy::test::make_client_ssl_ctx();

    return
      [&server, coro = asio::coroutine(),
       request  = std::make_unique<http::request<http::string_body>>(),
       response = std::make_unique<http::response<http::string_body>>(),
       client   = std::make_unique<foxy::client_session>(
         io.get_executor(), foxy::session_opts{client_ssl_ctx, std::chrono::seconds{30}}),
       client_req = std::make_unique<http::request<http::empty_body>>(),
       client_res = std::make_unique<http::response<http::string_body>>()](
        auto& self, boost::system::error_code ec = {}, std::size_t bytes_transferred = 0) mutable {
        reenter(coro)
        {
          server.opts.timeout = std::chrono::seconds{30};

          yield server.async_read(*request, std::move(self));
          if (ec) { return self.complete(ec, bytes_transferred); }

          if (request->method() != http::verb::post) {
            response->result(http::status::method_not_allowed);
            response->body() = "The server only supports POST requests!\n";
            response->prepare_payload();

            yield server.async_write(*response, std::move(self));
            return self.complete(ec, bytes_transferred);
          }

          if (request->body().size() == 0) {
            response->result(http::status::bad_request);
            response->body() = "The client body must not be empty!\n";
            response->prepare_payload();

            yield server.async_write(*response, std::move(self));
            return self.complete(ec, bytes_transferred);
          }

          client_req->method(http::verb::get);
          client_req->set(http::field::host, "www.google.com");
          {
            auto target = std::string("/webhp?");

            auto const view        = foxy::code_point_view<char>(request->body());
            auto const code_points = std::vector<char32_t>(view.begin(), view.end());
            foxy::uri::encode_query({code_points.data(), code_points.size()},
                                    std::back_inserter(target));

            client_req->target(target);
          }

          yield client->async_connect("www.google.com", "https", std::move(self));
          yield client->async_request(*client_req, *client_res, std::move(self));

          if (ec) {
            response->result(http::status::bad_request);
            response->body() = "Error communicating with google!\n";
            response->prepare_payload();

            yield server.async_write(*response, std::move(self));
            yield client->async_shutdown(std::move(self));
            return self.complete(ec, bytes_transferred);
          }

          if (client_res->result() != http::status::ok) {
            response->result(http::status::bad_request);
            response->body() = "Error is: " + client_res->body() + "\n";
          } else {
            response->result(http::status::ok);
            response->body() = client_res->body();
          }

          response->prepare_payload();
          yield server.async_write(*response, std::move(self));
          yield client->async_shutdown(std::move(self));

          self.complete({}, 0);
        }
      };
  });

  asio::spawn(io.get_executor(), [&](auto yield_ctx) mutable {
    auto client = foxy::client_session(io.get_executor(), {{}, std::chrono::seconds(30), false});
    client.async_connect("127.0.0.1", "1337", yield_ctx);

    auto req = http::request<http::string_body>(http::verb::post, "/", 11);
    auto res = http::response<http::string_body>();

    req.body() = "hello world in spanish";
    req.prepare_payload();

    client.async_request(req, res, yield_ctx);

    std::cout << "Got the response:\n" << res << "\n";

    auto ec = boost::system::error_code();
    client.async_shutdown(yield_ctx[ec]);

    listener.shutdown();
  });

  io.run();

  return 0;
}
