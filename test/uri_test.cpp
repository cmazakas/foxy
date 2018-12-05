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
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::sub_delims());
      });

    CHECK(matched_all_sub_delims);

    auto       view  = boost::string_view("rawr");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const non_match = !x3::parse(begin, end, foxy::uri::sub_delims());

    CHECK(non_match);
  }

  SECTION("should parse the gen-delims")
  {
    auto const delims =
      std::vector<boost::string_view>{":", "/", "?", "#", "[", "]", "@"};

    auto const matched_all_gen_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::gen_delims());
      });

    CHECK(matched_all_gen_delims);

    auto       view  = boost::string_view("rawr");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const non_match = !x3::parse(begin, end, foxy::uri::gen_delims());

    CHECK(non_match);
  }

  SECTION("should parse the reserved")
  {
    auto const delims = std::vector<boost::string_view>{
      ":", "/", "?", "#", "[", "]", "@", "!", "$",
      "&", "'", "(", ")", "*", "+", ",", ";", "="};

    auto const matched_all_reserved =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::reserved());
      });

    CHECK(matched_all_reserved);

    {
      auto       view  = boost::string_view("rawr");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const non_match = !x3::parse(begin, end, foxy::uri::reserved());

      CHECK(non_match);
    }

    {
      auto       view  = boost::string_view("~~~~Leonine.King1199__---");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match = x3::parse(begin, end, +foxy::uri::unreserved());

      CHECK(match);
      CHECK(begin == end);
    }
  }
}
