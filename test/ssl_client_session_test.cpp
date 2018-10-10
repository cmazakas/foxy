//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#include <foxy/client_session.hpp>
#include <foxy/log.hpp>

#include <boost/beast/version.hpp>
#include <boost/beast/http.hpp>
#include <boost/smart_ptr/make_unique.hpp>

#include <memory>

#include <catch2/catch.hpp>

using boost::system::error_code;
using boost::asio::ip::tcp;
namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace ssl  = boost::asio::ssl;

using namespace std::chrono_literals;

TEST_CASE("Our SSL client session class")
{
  SECTION("should be able to asynchronously connect to a remote")
  {
    asio::io_context io;

    // create a client that uses TLS 1.2 and has a 30 second timeout
    //
    auto ctx  = ssl::context(ssl::context::method::tlsv12_client);
    auto opts = foxy::session_opts{ctx, 30s};

    auto session_handle = boost::make_unique<foxy::client_session>(io, opts);
    auto& session       = *session_handle;

    auto valid_request = false;

    session.async_connect(
      "www.google.com", "https",
      [&valid_request, &session, sh = std::move(session_handle)]
      (error_code ec, tcp::endpoint) mutable -> void
      {
        auto parser_handle  = boost::make_unique<http::response_parser<http::string_body>>();
        auto request_handle =
          boost::make_unique<http::request<http::empty_body>>(http::verb::get, "/", 11);

        auto& parser  = *parser_handle;
        auto& request = *request_handle;

        session.async_request(
          request, parser,
          [
            &valid_request, &session, &parser, &request,
            ph = std::move(parser_handle),
            rh = std::move(request_handle),
            sh = std::move(sh)
          ](error_code ec) mutable -> void
          {
            auto response = parser.release();

            auto is_valid_status = (response.result_int() == 200);
            auto is_valid_body   = (response.body().size() > 0);

            valid_request = is_valid_body && is_valid_status;

            session
              .stream
              .ssl()
              .async_shutdown(
                [
                  &valid_request, &session, &parser, &request,
                  ph = std::move(ph),
                  rh = std::move(rh),
                  sh = std::move(sh)
                ]
                (error_code ec) -> void
                {
                  if (ec == boost::asio::error::eof) {
                    // Rationale:
                    // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
                    ec.assign(0, ec.category());
                  }

                  if (ec) { foxy::log_error(ec, "ssl client shutdown"); }
                });
          });
      });

    io.run();
    REQUIRE(valid_request);
  }
}
