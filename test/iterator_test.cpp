//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/iterator.hpp>

#include <boost/utility/string_view.hpp>
#include <boost/locale/utf.hpp>

#include <vector>
#include <algorithm>

#include <catch2/catch.hpp>

namespace utf = boost::locale::utf;

TEST_CASE("Our Unicode code point iterator...")
{
  SECTION("should be able to traverse a valid string and yield valid Unicode code points")
  {
    auto const input = boost::wstring_view(L"hello, world!");

    auto const expected = //       'h'  'e'  'l'  'l'  'o'  ',' ' ' 'w'  'o'  'r'  'l'  'd'  '!'
      std::vector<utf::code_point>{104, 101, 108, 108, 111, 44, 32, 119, 111, 114, 108, 100, 33};

    auto begin = foxy::uri::make_code_point_iterator(input.begin(), input.end());
    auto end   = foxy::uri::make_code_point_iterator(input.end(), input.end());

    auto const code_points = std::vector<utf::code_point>(begin, end);

    auto const ranges_match = std::equal(expected.begin(), expected.end(), code_points.begin());

    CHECK(ranges_match);
  }
}
