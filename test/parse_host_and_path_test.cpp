#include <foxy/detail/parse_host_and_path.hpp>

#include <boost/utility/string_view.hpp>

#include <tuple>
#include <string>
#include <functional>
#include <vector>
#include <algorithm>

#include <catch2/catch.hpp>

namespace x3 = boost::spirit::x3;

TEST_CASE("Our absolute URI parser...")
{
  SECTION("should match the sub delims")
  {
    auto const delims = std::vector<boost::string_view>{
      "!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="};

    auto const matched_all_sub_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        return x3::parse(delim.begin(), delim.end(), foxy::uri::sub_delims);
      });

    CHECK(matched_all_sub_delims);
  }

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
