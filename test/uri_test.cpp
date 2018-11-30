#include <foxy/uri.hpp>
#include <foxy/detail/uri_def.hpp>
#include <boost/utility/string_view.hpp>
#include <vector>
#include <algorithm>

#include <catch2/catch.hpp>

namespace x3 = boost::spirit::x3;

TEST_CASE("Our URI module...")
{
  SECTION("should parse the sub-delims")
  {
    auto const delims = std::vector<boost::string_view>{
      "!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="};

    auto const matched_all_sub_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        return x3::parse(delim.begin(), delim.end(), foxy::uri::sub_delims());
      });

    CHECK(matched_all_sub_delims);
  }
}
