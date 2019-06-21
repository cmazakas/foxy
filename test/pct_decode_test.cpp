#include <foxy/pct_decode.hpp>
#include <foxy/code_point_iterator.hpp>

#include <boost/utility/string_view.hpp>

#include <array>
#include <algorithm>

#include <catch2/catch.hpp>

TEST_CASE("Our pct-decoding function...")
{
  SECTION("should turn a pct-encoded ascii string into its underlying utf-8 representation")
  {
    auto const input = boost::string_view("www.%c5%bc%c3%b3%c5%82%c4%87.pl");

    auto bytes       = std::array<char, 256>{0};
    auto code_points = std::array<char32_t, 256>{0};

    auto const point_view = foxy::code_point_view<char>(
      boost::string_view(bytes.data(), foxy::pct_decode(input, bytes.begin()) - bytes.begin()));

    auto const decoded_view = boost::u32string_view(
      code_points.data(),
      std::copy(point_view.begin(), point_view.end(), code_points.begin()) - code_points.begin());

    CHECK(decoded_view == U"www.\u017C\u00F3\u0142\u0107.pl");
  }
}
