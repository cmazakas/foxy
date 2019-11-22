#define CATCH_CONFIG_MAIN

#include <boost/beast/http.hpp>

#include <catch2/catch.hpp>

namespace http = boost::beast::http;

TEST_CASE("niall_body_test")
{
  SECTION("niall_body_type should be able to send 1 million 128 byte buffers")
  {
    using body_type = http::empty_body;

    auto request = http::request<body_type>();
  }
}
