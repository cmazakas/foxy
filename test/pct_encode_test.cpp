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
  }
}
