#include "foxy/utility.hpp"

#include <catch2/catch.hpp>

TEST_CASE("Our utility library")
{
  SECTION("should parse the authority form of a uri for the host and port")
  {
    auto const uri           = "www.google.com:80";
    auto const host_and_port = foxy::parse_authority_form(uri);

    REQUIRE(std::get<0>(host_and_port) == "www.google.com");
    REQUIRE(std::get<1>(host_and_port) == "80");
  }
}
