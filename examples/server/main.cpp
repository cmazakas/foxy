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
    auto& f = *frame_ptr;
    reenter(*this)
    {
      while (true) {
        f.response = {};
        f.request  = {};

        yield server.async_read(f.request, std::move(self));
        if (ec) {
          std::cout << "Encountered error when reading in the request!\n" << ec << "\n";
          break;
        }

        std::cout << "Received message!\n" << f.request << "\n";

        f.response.result(http::status::ok);
        f.response.version(11);
        f.response.body() = "<html><p>Hello, world!</p></html>";
        f.response.prepare_payload();

        yield server.async_write(f.response, std::move(self));

        if (ec) {
          std::cout << "Encountered error when writing the response!\n" << ec << "\n";
          break;
        }

        if (!f.request.keep_alive()) { break; }
      }

      return self.complete({}, 0);
    }
  }
};

#include <boost/asio/unyield.hpp>

int
main()
{
  asio::io_context io{1};

  auto s = foxy::listener(io.get_executor(), tcp::endpoint(asio::ip::make_address("127.0.0.1"),
                                                           static_cast<unsigned short>(1337)));
  s.async_accept([](auto& server_session) { return request_handler(server_session); });

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
