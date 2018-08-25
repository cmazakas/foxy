#include "foxy/proxy.hpp"

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ip/address_v4.hpp>

#include <catch2/catch.hpp>

using boost::asio::ip::tcp;
namespace ip   = boost::asio::ip;
namespace asio = boost::asio;

TEST_CASE("Our forward proxy")
{
  SECTION("should reject all non-persistent connections")
  {
    asio::io_context io;

    auto const src_addr     = ip::make_address_v4("127.0.0.1");
    auto const src_port     = static_cast<unsigned short>(1337);
    auto const src_endpoint = tcp::endpoint(src_addr, src_port);

    auto const reuse_addr = false;

    auto proxy = foxy::proxy(io, src_endpoint, reuse_addr);
    proxy.async_accept();

    io.run();
  }
}
