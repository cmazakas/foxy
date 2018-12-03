#include <foxy/uri.hpp>
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
        auto       begin  = delim.begin();
        auto const end    = delim.end();
        auto       unused = x3::unused_type();
        return x3::parse(begin, end, foxy::uri::sub_delims(), unused);
      });

    CHECK(matched_all_sub_delims);

    auto       view   = boost::string_view("rawr");
    auto       begin  = view.begin();
    auto const end    = view.end();
    auto       unused = x3::unused_type();
    auto const non_match =
      !x3::parse(begin, end, foxy::uri::sub_delims(), unused);

    CHECK(non_match);
  }

  SECTION("should parse the gen-delims")
  {
    auto const delims =
      std::vector<boost::string_view>{":", "/", "?", "#", "[", "]", "@"};

    auto const matched_all_gen_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin  = delim.begin();
        auto const end    = delim.end();
        auto       unused = x3::unused_type();
        return x3::parse(begin, end, foxy::uri::gen_delims(), unused);
      });

    CHECK(matched_all_gen_delims);

    auto       view   = boost::string_view("rawr");
    auto       begin  = view.begin();
    auto const end    = view.end();
    auto       unused = x3::unused_type();
    auto const non_match =
      !x3::parse(begin, end, foxy::uri::gen_delims(), unused);

    CHECK(non_match);
  }
}
