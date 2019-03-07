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
#include <array>
#include <set>
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
    // We need to first check that we successfully do nothing to the ASCII set
    // In the case of UTF-8 encoding, encoding a 7 bit code point is equivalent to the identity
    // function
    //
    {
      auto can_encode_7_bits = true;
      for (utf::code_point code_point = 0x0; code_point < 0x80; ++code_point) {
        auto const is_same = (foxy::uri::to_utf8_encoding(code_point) == code_point);
        can_encode_7_bits  = can_encode_7_bits && is_same;
      }

      CHECK(can_encode_7_bits);
    }

    //
    // Now we test the next Unicode code point space, the 11 bit range
    // This begins at 0x0080 and ends at 0x07ff
    //
    {
      auto encoded_points = std::set<std::uint32_t>();

      auto can_encode_11_bits = true;
      for (utf::code_point code_point = 0x0080; code_point < 0x0800; ++code_point) {
        auto const encoded = foxy::uri::to_utf8_encoding(code_point);

        auto const bits = std::bitset<16>(encoded);

        std::cout << bits << "\n";

        //
        // we only need to test the seninel bits in practice
        // we also use the uniqueness check and the number of unique values to assert that we're
        // able to properly enumerate the entire 11 bit Unicode code point space into valid UTF-8
        //
        can_encode_11_bits =
          can_encode_11_bits && bits[15] && bits[14] && !bits[13] && bits[7] && !bits[6];

        if (can_encode_11_bits) { encoded_points.insert(encoded); }
      }

      CHECK(can_encode_11_bits);
      CHECK(encoded_points.size() == 0x0800 - 0x0080);
    }
  }
}
