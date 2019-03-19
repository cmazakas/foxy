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
#include <type_traits>

#include <catch2/catch.hpp>

namespace utf = boost::locale::utf;

TEST_CASE("Our Unicode code point iterator...")
{
  SECTION("should be able to traverse a valid string and yield valid Unicode code points")
  {
    auto const input = boost::wstring_view(L"hello, world!");

    auto const expected = //       'h'  'e'  'l'  'l'  'o'  ',' ' ' 'w'  'o'  'r'  'l'  'd'  '!'
      std::vector<utf::code_point>{104, 101, 108, 108, 111, 44, 32, 119, 111, 114, 108, 100, 33};

    auto const points_view  = foxy::uri::code_point_view<wchar_t>(input);
    auto const code_points  = std::vector<utf::code_point>(points_view.begin(), points_view.end());
    auto const ranges_match = std::equal(expected.begin(), expected.end(), code_points.begin());

    CHECK(ranges_match);
  }

  SECTION("should follow the proper Iterator requirements")
  {
    // CopyConstructible
    {
      foxy::uri::code_point_iterator<char const*> const pos{nullptr, nullptr};

      auto pos2(pos);
    }

    // MoveConstructible
    {
      foxy::uri::code_point_iterator<char const*> pos{nullptr, nullptr};

      auto pos2(std::move(pos));
      auto pos3(foxy::uri::code_point_iterator<char const*>{nullptr, nullptr});
    }

    // CopyAssignable
    {
      foxy::uri::code_point_iterator<char const*> pos{nullptr, nullptr};
      foxy::uri::code_point_iterator<char const*> pos2{nullptr, nullptr};

      pos2 = pos;
    }

    // MoveAssignable
    {
      foxy::uri::code_point_iterator<char const*> pos{nullptr, nullptr};
      foxy::uri::code_point_iterator<char const*> pos2{nullptr, nullptr};

      pos2 = std::move(pos);
    }

    // Destructible is proven by previous code compiling...

    // Swappable
    // Swappable in the case of Foxy's code_point_iterator will be essentially swapping the views
    // owned by the other
    // So this means, they should point to different strings (or different positions in the same
    // view) and performing a swap will reset their traversal to the other's
    //
    {
      using std::swap;
      using foxy::uri::code_point::swap;

      auto input1 = boost::u16string_view(u"hello, world!");
      auto input2 = boost::u16string_view(u"hello, world!");

      auto const code_points = //       'h'  'e'  'l'  'l'  'o'  ',' ' ' 'w'  'o'  'r'  'l'  'd' '!'
        std::vector<utf::code_point>{104, 101, 108, 108, 111, 44, 32, 119, 111, 114, 108, 100, 33};

      auto const pts_view1 = foxy::uri::code_point_view<char16_t>(input1);
      auto const pts_view2 = foxy::uri::code_point_view<char16_t>(input2);

      auto begin1 = pts_view1.begin();
      auto begin2 = pts_view2.begin();

      CHECK(*begin1++ == 104);

      CHECK(*begin2++ == 104);
      CHECK(*begin2++ == 101);
      CHECK(*begin2++ == 108);

      swap(begin1, begin2);

      CHECK(*begin1++ == 108);
      CHECK(*begin1++ == 111);

      CHECK(*begin2++ == 101);
      CHECK(*begin2++ == 108);
      CHECK(*begin2++ == 108);
      CHECK(*begin2++ == 111);
    }

    // EqualityComparable
    //
    {
      auto input = boost::u16string_view(u"hello, world!");

      auto points_view = foxy::uri::code_point_view<char16_t>(input);

      auto iter1 = points_view.begin();
      auto iter2 = points_view.begin();

      auto end = points_view.end();

      CHECK(iter1 == iter2);
      CHECK(iter1 != end);
      CHECK(iter2 != end);
    }

    // ensure `std::iterator_traits` has what it needs
    //
    {
      static_assert(
        std::is_same<
          typename std::iterator_traits<foxy::uri::code_point_iterator<char const*>>::value_type,
          utf::code_point>::value,
        "Invalid value_type");

      static_assert(std::is_same<typename std::iterator_traits<
                                   foxy::uri::code_point_iterator<char const*>>::difference_type,
                                 std::ptrdiff_t>::value,
                    "Invalid difference_type");

      static_assert(
        std::is_same<
          typename std::iterator_traits<foxy::uri::code_point_iterator<char const*>>::reference,
          utf::code_point>::value,
        "Invalid reference type");

      static_assert(
        std::is_same<
          typename std::iterator_traits<foxy::uri::code_point_iterator<char const*>>::pointer,
          void>::value,
        "Invalid pointer type");

      static_assert(std::is_same<typename std::iterator_traits<
                                   foxy::uri::code_point_iterator<char const*>>::iterator_category,
                                 std::input_iterator_tag>::value,
                    "Invalid pointer type");
    }
  }
}
