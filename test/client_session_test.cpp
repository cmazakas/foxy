#include "foxy/client_session.hpp"

#include <boost/beast/http.hpp>
#include <boost/smart_ptr/make_unique.hpp>

#include <memory>

#include <catch2/catch.hpp>

using boost::system::error_code;
using boost::asio::ip::tcp;
namespace asio = boost::asio;
namespace http = boost::beast::http;

TEST_CASE("Our client session class")
{
  SECTION("should be able to asynchronously connect to a remote")
  {
    asio::io_context io;

    auto  session_handle = boost::make_unique<foxy::client_session>(io);
    auto& session = *session_handle;

    auto valid_request = false;

    session.async_connect(
      "www.google.com", "http",
      [&valid_request, &session, sh = std::move(session_handle)]
      (error_code ec, tcp::endpoint) mutable -> void
      {
        if (ec) {
          session
            .stream
            .tcp()
            .shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
          return;
        }

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
          ](error_code ec) -> void
          {
            auto response = parser.release();

            auto is_valid_status = (response.result_int() == 200);
            auto is_valid_body   = (response.body().size() > 0);

            valid_request = is_valid_body && is_valid_status;

            session
              .stream
              .tcp()
              .shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
          });
      });

    io.run();
    REQUIRE(valid_request);
  }
}