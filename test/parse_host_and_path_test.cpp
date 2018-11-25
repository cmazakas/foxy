#include <foxy/detail/parse_host_and_path.hpp>
#include <boost/utility/string_view.hpp>
#include <tuple>
#include <string>

#include <catch2/catch.hpp>

TEST_CASE("Our absolute URI parser...")
{
  SECTION("should extract the host, port and target")
  {
    auto const uri = boost::string_view("http://www.google.com:1337/");

    auto host   = std::string();
    auto port   = std::string();
    auto target = std::string();

    CHECK(host == "www.google.com");
    CHECK(port == "1337");
    CHECK(target == "/");
  }
}
