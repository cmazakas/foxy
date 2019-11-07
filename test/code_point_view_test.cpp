//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/code_point_iterator.hpp>
#include <foxy/pct_encode.hpp>
#include <foxy/parse_uri.hpp>

#include <boost/utility/string_view.hpp>

#include <vector>
#include <algorithm>
#include <type_traits>
#include <array>
#include <cstring>

#include <catch2/catch.hpp>

TEST_CASE("code_point_view_test")
{
  SECTION("should interop with our Unicode URI parsers")
  {
    auto url = boost::u16string_view(
      u"http://www.google.com/?q=\u0412\u0441\u0435\u043c+\u043f\u0440\u0438\u0432\u0435\u0442+c");

    auto point_view = foxy::code_point_view<char16_t>(url);

    auto const code_points = std::vector<char32_t>(point_view.begin(), point_view.end());
    auto const unicode_url = boost::u32string_view(code_points.data(), code_points.size());

    auto const uri_parts = foxy::parse_uri(unicode_url);

    CHECK(uri_parts.scheme() == U"http");
    CHECK(uri_parts.host() == U"www.google.com");
    CHECK(uri_parts.path() == U"/");
    CHECK(uri_parts.query() ==
          U"q=\u0412\u0441\u0435\u043c+\u043f\u0440\u0438\u0432\u0435\u0442+c");
    CHECK(uri_parts.fragment() == U"");
  }

  SECTION("[wide] should interop with our utf-8 encoder")
  {
    auto input      = boost::wstring_view(L"\u20ac");
    auto point_view = foxy::code_point_view<wchar_t>(input);
    auto buff       = std::array<unsigned char, 3>{0};

    foxy::utf8_encode(point_view.begin(), point_view.end(), buff.begin());

    CHECK(buff[0] == 0xe2);
    CHECK(buff[1] == 0x82);
    CHECK(buff[2] == 0xac);
  }

  SECTION("[utf-32] should interop with our utf-8 encoder")
  {
    auto input      = boost::u32string_view(U"\u20ac");
    auto point_view = foxy::code_point_view<char32_t>(input);
    auto buff       = std::array<unsigned char, 3>{0};

    foxy::utf8_encode(point_view.begin(), point_view.end(), buff.begin());

    CHECK(buff[0] == 0xe2);
    CHECK(buff[1] == 0x82);
    CHECK(buff[2] == 0xac);
  }

  SECTION("[utf-16] should interop with our utf-8 encoder")
  {
    auto input      = boost::u16string_view(u"\u20ac");
    auto point_view = foxy::code_point_view<char16_t>(input);
    auto buff       = std::array<unsigned char, 3>{0};

    foxy::utf8_encode(point_view.begin(), point_view.end(), buff.begin());

    CHECK(buff[0] == 0xe2);
    CHECK(buff[1] == 0x82);
    CHECK(buff[2] == 0xac);
  }

  SECTION("[utf-8] should interop with our utf-8 encoder")
  {
    auto input      = boost::string_view(u8"\u20ac");
    auto point_view = foxy::code_point_view<char>(input);
    auto buff       = std::array<unsigned char, 3>{0};

    foxy::utf8_encode(point_view.begin(), point_view.end(), buff.begin());

    CHECK(buff[0] == 0xe2);
    CHECK(buff[1] == 0x82);
    CHECK(buff[2] == 0xac);
  }
}
