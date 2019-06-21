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
      boost::string_view(bytes.data(), foxy::uri::pct_decode(input, bytes.begin()) - bytes.begin());

    CHECK(decoded_view == u8"www.\u017C\u00F3\u0142\u0107.pl");
  }

  SECTION("should not pct-decode the sub-delims")
  {
    auto const input = boost::string_view("!$'()*+,;=");

    auto bytes = std::array<char, 256>{0};

    auto const decoded_view =
      boost::string_view(bytes.data(), foxy::uri::pct_decode(input, bytes.begin()) - bytes.begin());

    CHECK(decoded_view == u8"!$'()*+,;=");
  }

  SECTION("should pct-decode whitespace")
  {
    auto const input = boost::string_view("hello%20world!%0a");

    auto bytes = std::array<char, 256>{0};

    auto const decoded_view =
      boost::string_view(bytes.data(), foxy::uri::pct_decode(input, bytes.begin()) - bytes.begin());

    CHECK(decoded_view == u8"hello world!\n");
  }

  SECTION("should pct-decode misc. ascii chars")
  {
    auto const input = boost::string_view("%22%23%2f%3c%3e%3f%40%5b%5c%5d%5e%60%7b%7c%7d");

    auto bytes = std::array<char, 256>{0};

    auto const decoded_view =
      boost::string_view(bytes.data(), foxy::uri::pct_decode(input, bytes.begin()) - bytes.begin());

    CHECK(decoded_view == u8"\"#/<>?@[\\]^`{|}");
  }

  SECTION("should not pct-decode the ALPHA | DIGIT | -._~ | sub-delims")
  {
    auto const input = boost::string_view(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");

    auto bytes = std::array<char, 256>{0};

    auto const decoded_view =
      boost::string_view(bytes.data(), foxy::uri::pct_decode(input, bytes.begin()) - bytes.begin());

    CHECK(decoded_view ==
          u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");
  }

  SECTION("should not pct-decode select chars")
  {
    auto const input = boost::string_view("/%22%23%3c%3e%3f@%5b%5c%5d%5e%60%7b%7c%7d:::");

    auto bytes = std::array<char, 256>{0};

    auto const decoded_view =
      boost::string_view(bytes.data(), foxy::uri::pct_decode(input, bytes.begin()) - bytes.begin());

    CHECK(decoded_view == u8"/\"#<>?@[\\]^`{|}:::");
  }
}
