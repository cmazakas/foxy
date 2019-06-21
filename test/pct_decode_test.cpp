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

    auto bytes = std::array<char, 256>{0};

    auto const decoded_view =
      boost::string_view(bytes.data(), foxy::pct_decode(input, bytes.begin()) - bytes.begin());

    CHECK(decoded_view == u8"www.\u017C\u00F3\u0142\u0107.pl");
  }
}
