#include <foxy/detail/tunnel.hpp>
#include <foxy/session.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/experimental/test/stream.hpp>

#include <catch2/catch.hpp>

namespace asio  = boost::asio;
namespace beast = boost::beast;
namespace http  = boost::beast::http;

TEST_CASE("Our async tunnel operation")
{
  SECTION("should enable absolute URIs as a valid test target")
  {

  }
}
