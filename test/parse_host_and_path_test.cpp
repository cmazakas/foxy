#include <foxy/detail/parse_host_and_path.hpp>

#include <boost/utility/string_view.hpp>

#include <tuple>
#include <string>
#include <functional>

#include <catch2/catch.hpp>

TEST_CASE("Our absolute URI parser...")
{
  SECTION("should extract the host, port and target")
  {
    auto const uri = boost::string_view("http://www.google.com:1337/");

    auto host   = std::string();
    auto port   = std::string();
    auto target = std::string();

    host.reserve(32);
    port.reserve(32);
    target.reserve(32);

    auto attribute =
      std::make_tuple(std::ref(host), std::ref(port), std::ref(target));

    foxy::detail::parse_host_and_path(uri, attribute);

    CHECK(host == "www.google.com");
    CHECK(port == "1337");
    CHECK(target == "/");
  }
}
