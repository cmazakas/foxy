//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/client_session.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/core/bind_handler.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/coroutine.hpp>

#include <iostream>
#include <memory>
#include <string>

namespace asio = boost::asio;
namespace http = boost::beast::http;

#include <boost/asio/yield.hpp>
struct request_op : asio::coroutine
{
  using executor_type = asio::io_context::executor_type;

  struct frame
  {
    http::request<http::empty_body>   request = {http::verb::get, "/", 11};
    http::response<http::string_body> response;

    std::string host    = "google.com";
    std::string service = "http";
  };

  foxy::client_session&  client;
  executor_type          executor;
  std::unique_ptr<frame> p = std::make_unique<frame>();

  request_op()                  = delete;
  request_op(request_op const&) = delete;
  request_op(request_op&&)      = default;

  request_op(foxy::client_session& client_, executor_type executor_)
    : client(client_)
    , executor(executor_)
  {
  }

  struct on_async_completion_t
  {
  };

  auto
  get_executor() const noexcept -> executor_type
  {
    return executor;
  }

  template <class... Args>
  auto
  operator()(on_async_completion_t, boost::system::error_code ec, Args&&...) -> void
  {
    (*this)(ec);
  }

  auto operator()(boost::system::error_code ec = {}) -> void
  {
    auto& s = *p;
    reenter(*this)
    {
      s.request.set(http::field::host, "www.google.com");

      yield client.async_connect(
        s.host, s.service,
        boost::beast::bind_front_handler(std::move(*this), on_async_completion_t{}));
      if (ec) { goto upcall; }

      yield client.async_request(s.request, s.response, std::move(*this));
      if (ec) { goto upcall; }

      {
        auto& socket = client.stream.plain();
        socket.shutdown(asio::ip::tcp::socket::shutdown_both);
        socket.close();
      }

      std::cout << "Got a response back from Google!\n";
      std::cout << s.response << "\n\n";

    upcall:
      if (ec) {
        std::cout << "Error: " << ec.message() << "\n\n";
        return;
      }
    }
  }
};
#include <boost/asio/unyield.hpp>

int
main()
{
  asio::io_context io{1};

  auto client = foxy::client_session(io, {});
  asio::post(io, request_op(client, io.get_executor()));
  io.run();

  return 0;
}
