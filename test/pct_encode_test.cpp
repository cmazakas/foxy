//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/pct_encode.hpp>

#include <boost/locale/utf.hpp>

#include <string>
#include <cstdint>
#include <iterator>
#include <vector>
#include <bitset>

#include <iostream>

#include <catch2/catch.hpp>

namespace utf = boost::locale::utf;

TEST_CASE("Our percent encoding function/namespace should...")
{
  SECTION(
    "... should be able to convert any Unicode code point to its underlying UTF-8 binary "
    "representation")
  {
    //
    // [U+0000,  U+007F]
    //
    {
      auto all_ascii = std::string();
      all_ascii.reserve(128);

      for (std::uint8_t unicode_char = 0x00; unicode_char < 0x80; ++unicode_char) {
        all_ascii.push_back(unicode_char);
      }

      auto out = std::string(128, '\0');
      auto end = foxy::uri::utf8_encoding(all_ascii.begin(), all_ascii.end(), out.begin());

      CHECK(std::distance(out.begin(), end) == 128);
      CHECK(out == all_ascii);
    }

    //
    // [U+0080, U+0800]
    //
    {
      auto code_points = std::vector<utf::code_point>();
      code_points.reserve(0x0800 - 0x0080);
      for (utf::code_point code_point = 0x0080; code_point < 0x0800; ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<std::uint8_t>();
      utf8_bytes.resize(2 * code_points.size());

      foxy::uri::utf8_encoding(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<32>(code_points[idx]);

        auto const first_byte  = std::bitset<8>(utf8_bytes[2 * idx]);
        auto const second_byte = std::bitset<8>(utf8_bytes[(2 * idx) + 1]);

        is_valid_encoding = is_valid_encoding && first_byte[7];
        is_valid_encoding = is_valid_encoding && first_byte[6];
        is_valid_encoding = is_valid_encoding && !first_byte[5];
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(first_byte[4]) == static_cast<bool>(code_point[10]));

        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(first_byte[3]) == static_cast<bool>(code_point[9]));
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(first_byte[2]) == static_cast<bool>(code_point[8]));
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(first_byte[1]) == static_cast<bool>(code_point[7]));
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(first_byte[0]) == static_cast<bool>(code_point[6]));

        is_valid_encoding = is_valid_encoding && second_byte[7];
        is_valid_encoding = is_valid_encoding && !second_byte[6];
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(second_byte[5]) == static_cast<bool>(code_point[5]));
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(second_byte[4]) == static_cast<bool>(code_point[4]));

        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(second_byte[3]) == static_cast<bool>(code_point[3]));
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(second_byte[2]) == static_cast<bool>(code_point[2]));
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(second_byte[1]) == static_cast<bool>(code_point[1]));
        is_valid_encoding = is_valid_encoding &&
                            (static_cast<bool>(second_byte[0]) == static_cast<bool>(code_point[0]));
      }

      CHECK(is_valid_encoding);
    }
  }
}
