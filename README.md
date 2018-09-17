# Foxy
Session-based Beast/Asio wrapper requiring C++14

## High-Level Overview

Foxy is a humble wrapper around Boost.Beast/Boost.Asio and emphasizes a
session-focused API.

The user is given 3 versions of an HTTP session: a `client_session`,
`server_session` and a generic `session` that is intended to be inherited from.

## Example Usage

```
asio::io_context io;

auto session_handle =
  boost::make_unique<foxy::client_session>(io);

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
        .plain()
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
          .plain()
          .shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      });
  });

io.run();
REQUIRE(valid_request);
```

## Current Status and Roadmap

Project is in its infantile settings but the `client_session` and
`server_session` classes are usable.

Initial minimum viable product is a proxy-aware HTTP client and a small proxy
server implementation.

