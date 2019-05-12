//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/pct_encode.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/locale/utf.hpp>

#include <string>
#include <cstdint>
#include <iterator>
#include <vector>
#include <bitset>
#include <array>
#include <iostream>

#include <catch2/catch.hpp>

namespace utf = boost::locale::utf;
namespace x3  = boost::spirit::x3;

TEST_CASE("Our percent encoding function/namespace...")
{
  SECTION(
    "... should be able to convert any Unicode code point to its underlying UTF-8 binary "
    "representation")
  {
    // [U+0000,  U+007F]
    //
    {
      auto const bytes_per_code_point = 1;
      auto const code_point_begin     = utf::code_point{0x0000};
      auto const code_point_end       = utf::code_point{0x0080};

      auto code_points = std::vector<utf::code_point>();
      code_points.reserve(code_point_end - code_point_begin);
      for (utf::code_point code_point = code_point_begin; code_point < code_point_end;
           ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<std::uint8_t>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::uri::utf8_encoding(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<8>(code_points[idx]);

        auto const first_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);

        is_valid_encoding = is_valid_encoding && !first_byte[7];
        is_valid_encoding = is_valid_encoding && (first_byte[6] == code_point[6]);
        is_valid_encoding = is_valid_encoding && (first_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (first_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (first_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }

    // [U+0080, U+0800]
    //
    {
      auto const bytes_per_code_point = 2;
      auto const code_point_begin     = utf::code_point{0x0080};
      auto const code_point_end       = utf::code_point{0x0800};

      auto code_points = std::vector<utf::code_point>();
      code_points.reserve(code_point_end - code_point_begin);
      for (utf::code_point code_point = code_point_begin; code_point < code_point_end;
           ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<std::uint8_t>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::uri::utf8_encoding(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<32>(code_points[idx]);

        auto const first_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);
        auto const second_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 1]);

        is_valid_encoding = is_valid_encoding && first_byte[7];
        is_valid_encoding = is_valid_encoding && first_byte[6];
        is_valid_encoding = is_valid_encoding && !first_byte[5];
        is_valid_encoding = is_valid_encoding && (first_byte[4] == code_point[10]);
        is_valid_encoding = is_valid_encoding && (first_byte[3] == code_point[9]);
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[8]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[7]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[6]);

        is_valid_encoding = is_valid_encoding && second_byte[7];
        is_valid_encoding = is_valid_encoding && !second_byte[6];
        is_valid_encoding = is_valid_encoding && (second_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (second_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (second_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (second_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (second_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (second_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }

    // [U+0800, U+10000]
    //
    {
      auto const bytes_per_code_point = 3;
      auto const code_point_begin     = utf::code_point{0x0800};
      auto const code_point_end       = utf::code_point{0x10000};

      auto code_points = std::vector<utf::code_point>();
      code_points.reserve(code_point_end - code_point_begin);
      for (utf::code_point code_point = code_point_begin; code_point < code_point_end;
           ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<std::uint8_t>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::uri::utf8_encoding(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<16>(code_points[idx]);

        auto const first_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);
        auto const second_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 1]);
        auto const third_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 2]);

        is_valid_encoding = is_valid_encoding && first_byte[7];
        is_valid_encoding = is_valid_encoding && first_byte[6];
        is_valid_encoding = is_valid_encoding && first_byte[5];
        is_valid_encoding = is_valid_encoding && !first_byte[4];
        is_valid_encoding = is_valid_encoding && (first_byte[3] == code_point[15]);
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[14]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[13]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[12]);

        is_valid_encoding = is_valid_encoding && second_byte[7];
        is_valid_encoding = is_valid_encoding && !second_byte[6];
        is_valid_encoding = is_valid_encoding && (second_byte[5] == code_point[11]);
        is_valid_encoding = is_valid_encoding && (second_byte[4] == code_point[10]);
        is_valid_encoding = is_valid_encoding && (second_byte[3] == code_point[9]);
        is_valid_encoding = is_valid_encoding && (second_byte[2] == code_point[8]);
        is_valid_encoding = is_valid_encoding && (second_byte[1] == code_point[7]);
        is_valid_encoding = is_valid_encoding && (second_byte[0] == code_point[6]);

        is_valid_encoding = is_valid_encoding && third_byte[7];
        is_valid_encoding = is_valid_encoding && !third_byte[6];
        is_valid_encoding = is_valid_encoding && (third_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (third_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (third_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (third_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (third_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (third_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }

    // [U+10000, U+110000]
    //
    {
      auto const bytes_per_code_point = 4;
      auto const code_point_begin     = utf::code_point{0x10000};
      auto const code_point_end       = utf::code_point{0x110000};

      auto code_points = std::vector<utf::code_point>();
      code_points.reserve(code_point_end - code_point_begin);
      for (utf::code_point code_point = code_point_begin; code_point < code_point_end;
           ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<std::uint8_t>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::uri::utf8_encoding(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<21>(code_points[idx]);

        auto const first_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);
        auto const second_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 1]);
        auto const third_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 2]);
        auto const fourth_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 3]);

        is_valid_encoding = is_valid_encoding && first_byte[7];
        is_valid_encoding = is_valid_encoding && first_byte[6];
        is_valid_encoding = is_valid_encoding && first_byte[5];
        is_valid_encoding = is_valid_encoding && first_byte[4];
        is_valid_encoding = is_valid_encoding && !first_byte[3];
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[20]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[19]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[18]);

        is_valid_encoding = is_valid_encoding && second_byte[7];
        is_valid_encoding = is_valid_encoding && !second_byte[6];
        is_valid_encoding = is_valid_encoding && (second_byte[5] == code_point[17]);
        is_valid_encoding = is_valid_encoding && (second_byte[4] == code_point[16]);
        is_valid_encoding = is_valid_encoding && (second_byte[3] == code_point[15]);
        is_valid_encoding = is_valid_encoding && (second_byte[2] == code_point[14]);
        is_valid_encoding = is_valid_encoding && (second_byte[1] == code_point[13]);
        is_valid_encoding = is_valid_encoding && (second_byte[0] == code_point[12]);

        is_valid_encoding = is_valid_encoding && third_byte[7];
        is_valid_encoding = is_valid_encoding && !third_byte[6];
        is_valid_encoding = is_valid_encoding && (third_byte[5] == code_point[11]);
        is_valid_encoding = is_valid_encoding && (third_byte[4] == code_point[10]);
        is_valid_encoding = is_valid_encoding && (third_byte[3] == code_point[9]);
        is_valid_encoding = is_valid_encoding && (third_byte[2] == code_point[8]);
        is_valid_encoding = is_valid_encoding && (third_byte[1] == code_point[7]);
        is_valid_encoding = is_valid_encoding && (third_byte[0] == code_point[6]);

        is_valid_encoding = is_valid_encoding && fourth_byte[7];
        is_valid_encoding = is_valid_encoding && !fourth_byte[6];
        is_valid_encoding = is_valid_encoding && (fourth_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }
  }

  SECTION("...should actually pct encode")
  {
    {
      auto const str = boost::wstring_view(L"\u20ac");
      auto       out = std::string(256, '\0');

      auto const out_end = foxy::uri::pct_encode(str, out.begin());

      auto const out_view = boost::string_view(out.data(), out_end - out.begin());

      CHECK(out_view == "%e2%82%ac");
    }

    {
      auto const str = boost::string_view(u8"\u20ac");
      auto       out = std::string(256, '\0');

      auto const out_end = foxy::uri::pct_encode(str, out.begin());

      auto const out_view = boost::string_view(out.data(), out_end - out.begin());

      CHECK(out_view == "%e2%82%ac");
    }

    {
      auto const str = boost::u16string_view(u"\u20ac");
      auto       out = std::string(256, '\0');

      auto const out_end = foxy::uri::pct_encode(str, out.begin());

      auto const out_view = boost::string_view(out.data(), out_end - out.begin());

      CHECK(out_view == "%e2%82%ac");
    }

    {
      auto const str = boost::u32string_view(U"\u20ac");
      auto       out = std::string(256, '\0');

      auto const out_end = foxy::uri::pct_encode(str, out.begin());

      auto const out_view = boost::string_view(out.data(), out_end - out.begin());

      CHECK(out_view == "%e2%82%ac");
    }
  }

  SECTION("should properly encode portions of the 7-bit ASCII set")
  {
    auto ascii_chars = std::vector<char>(128);
    for (int idx = 0; idx < 128; ++idx) { ascii_chars[idx] = static_cast<char>(idx); }

    auto test_views = std::vector<boost::string_view>(128);
    for (int idx = 0; idx < 128; ++idx) {
      test_views[idx] = boost::string_view(ascii_chars.data() + idx, 1);
    }

    auto was_encoded = true;
    for (int idx = 0; idx < 33; ++idx) {
      auto const test_view = test_views[idx];

      auto out = std::array<char, 3>{0};

      auto const out_end = foxy::uri::pct_encode(test_view, out.data());

      unsigned parsed_num = 0;
      auto     pos        = out.begin();
      x3::parse(pos, out.end(), "%" >> x3::hex, parsed_num);

      was_encoded == was_encoded && (parsed_num == ascii_chars[idx]);
    }

    CHECK(was_encoded);
  }
}
