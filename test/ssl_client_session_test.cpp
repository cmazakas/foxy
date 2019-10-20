//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/client_session.hpp>
#include <foxy/log.hpp>

#include <boost/asio/ssl/rfc2818_verification.hpp>
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

TEST_CASE("ssl_client_session_test")
{
  SECTION("should be able to asynchronously connect to a remote")
  {
    asio::io_context io{1};

    auto ctx = foxy::make_ssl_ctx(ssl::context::method::tlsv12_client);

    // load in our entire list of root CAs
    //
    ctx.load_verify_file("root-cas.pem");

    auto opts = foxy::session_opts{ctx, 30s};

    // create a client that uses TLS 1.2 and has a 30 second timeout
    //
    auto  session_handle = boost::make_unique<foxy::client_session>(io.get_executor(), opts);
    auto& session        = *session_handle;

    REQUIRE(session.stream.is_ssl());

    session.async_connect(
      "www.google.com", "https",
      [&session, sh = std::move(session_handle)](error_code ec) mutable -> void {
        REQUIRE_FALSE(ec);

        auto parser_handle = boost::make_unique<http::response_parser<http::string_body>>();
        auto request_handle =
          boost::make_unique<http::request<http::empty_body>>(http::verb::get, "/", 11);

        auto& parser  = *parser_handle;
        auto& request = *request_handle;

        session.async_request(
          request, parser,
          [&session, &parser, &request, ph = std::move(parser_handle),
           rh = std::move(request_handle), sh = std::move(sh)](error_code ec) mutable -> void {
            REQUIRE_FALSE(ec);

            auto response = parser.release();

            auto const is_valid_status = (response.result_int() == 200);
            auto const is_valid_body   = (response.body().size() > 0) &&
                                       boost::string_view(response.body()).ends_with("</html>");

            CHECK(is_valid_status);
            CHECK(is_valid_body);

            session.async_shutdown(
              [&session, sh = std::move(sh)](error_code ec) mutable -> void {});
          });
      });

    io.run();
  }

  SECTION("should timeout when the host can't be found")
  {
    asio::io_context io{1};

    auto ctx  = foxy::make_ssl_ctx(ssl::context::method::tlsv12_client);
    auto opts = foxy::session_opts{ctx, 250ms};

    auto  session_handle = boost::make_unique<foxy::client_session>(io.get_executor(), opts);
    auto& session        = *session_handle;

    auto timed_out = false;

    session.async_connect(
      "www.google.com", "1337",
      [&timed_out, &session, sh = std::move(session_handle)](error_code ec) mutable -> void {
        timed_out = (ec == boost::asio::error::operation_aborted);
      });

    io.run();
    REQUIRE(timed_out);
  }
}
