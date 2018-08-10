#include "foxy/client_session.hpp"

#include <boost/smart_ptr/make_unique.hpp>
#include <memory>
#include <iostream>

#include <catch2/catch.hpp>

using boost::system::error_code;
using boost::asio::ip::tcp;
namespace asio = boost::asio;

TEST_CASE("Our client session class")
{
  SECTION("should be able to asynchronously connect to a remote")
  {
    asio::io_context io;

    auto  session_handle = boost::make_unique<foxy::client_session>(io);
    auto& session = *session_handle;

    auto connected = false;

    session.async_connect(
      "www.google.com", "80",
      [&session, &connected, sh = std::move(session_handle)]
      (error_code ec, tcp::endpoint) -> void
      {
        if (!ec) {
          connected = true;
        }
        session.tcp().shutdown(boost::asio::ip::tcp::socket::shutdown_send, ec);
      });

    io.run();

    REQUIRE(connected);
  }
}