#include <foxy/pct_decode.hpp>
#include <foxy/uri.hpp>

#include <boost/utility/string_view.hpp>

#include <array>
#include <algorithm>

#include <catch2/catch.hpp>

namespace x3 = boost::spirit::x3;

TEST_CASE("pct_decode_test")
{
  SECTION("should turn a pct-encoded ascii string into its underlying utf-8 representation")
  {
    auto const input = boost::string_view("www.%c5%bc%c3%b3%c5%82%c4%87.pl");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"www.\u017C\u00F3\u0142\u0107.pl");
    CHECK_FALSE(ec);
  }

  SECTION("should not pct-decode the sub-delims")
  {
    auto const input = boost::string_view("!$'()*+,;=");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"!$'()*+,;=");
    CHECK_FALSE(ec);
  }

  SECTION("should pct-decode whitespace")
  {
    auto const input = boost::string_view("hello%20world!%0a");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"hello world!\n");
    CHECK_FALSE(ec);
  }

  SECTION("should pct-decode misc. ascii chars")
  {
    auto const input = boost::string_view("%22%23%2f%3c%3e%3f%40%5b%5c%5d%5e%60%7b%7c%7d");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"\"#/<>?@[\\]^`{|}");
    CHECK_FALSE(ec);
  }

  SECTION("should not pct-decode the ALPHA | DIGIT | -._~ | sub-delims")
  {
    auto const input = boost::string_view(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view ==
          u8"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");
    CHECK_FALSE(ec);
  }

  SECTION("should not pct-decode select chars")
  {
    auto const input = boost::string_view("/%22%23%3c%3e%3f@%5b%5c%5d%5e%60%7b%7c%7d:::");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"/\"#<>?@[\\]^`{|}:::");
    CHECK_FALSE(ec);
  }

  SECTION("should recognize malformed input and simply stop parsing")
  {
    auto ec = boost::system::error_code{};

    auto const invalid_host = boost::string_view("www.goo%%%%gle.com");

    auto pos = invalid_host.begin();
    x3::parse(pos, invalid_host.end(), foxy::uri::host);

    auto const was_full_match = (pos == invalid_host.end());
    REQUIRE(!was_full_match);

    auto bytes = std::array<char, 256>{0};

    auto decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(invalid_host, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == "www.goo");
    CHECK(ec == foxy::error::unexpected_pct);

    bytes.fill(0);

    auto out = bytes.begin();
    CHECK_THROWS(foxy::uri::pct_decode(invalid_host, out));

    decoded_view = boost::string_view(bytes.data(), out - bytes.begin());
    CHECK(decoded_view == "www.goo");
  }

  SECTION("should handle an incomplete pct-encoded byte")
  {
    auto const input = boost::string_view("%2");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"");
    CHECK(ec == foxy::error::unexpected_pct);
  }

  SECTION("should handle an invalid pct-encoded byte")
  {
    auto input = boost::string_view("%zz");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"");
    CHECK(ec == foxy::error::unexpected_pct);

    bytes.fill('\0');

    input = boost::string_view("%1g");

    decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"");
    CHECK(ec == foxy::error::unexpected_pct);
  }

  SECTION("should ignore 8 bit ASCII and other non-printables [i.e. no URI validation here]")
  {
    auto const input = boost::string_view("\x00\xff\x8f\x01");

    auto bytes = std::array<char, 256>{0};
    auto ec    = boost::system::error_code();

    auto const decoded_view = boost::string_view(
      bytes.data(), foxy::uri::pct_decode(input, bytes.begin(), ec) - bytes.begin());

    CHECK(decoded_view == u8"\x00\xff\x8f\x01");
    CHECK_FALSE(ec);
  }
}
