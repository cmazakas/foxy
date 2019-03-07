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
#include <catch2/catch.hpp>

namespace utf = boost::locale::utf;

TEST_CASE("Our percent encoding function/namespace should...")
{
  SECTION(
    "... should be able to convert any Unicode code point to its underlying UTF-8 binary "
    "representation")
  {
    {
      auto is_valid_utf_binary_rep = true;
      for (utf::code_point code_point = 0x0; code_point < 0x80; ++code_point) {
        auto const is_same      = (foxy::uri::to_utf8_encoding(code_point) == code_point);
        is_valid_utf_binary_rep = is_valid_utf_binary_rep && is_same;
      }

      CHECK(is_valid_utf_binary_rep);
    }
  }
}
