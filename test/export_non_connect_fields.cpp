//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#include <foxy/detail/export_non_connect_fields.hpp>
#include <boost/beast/http.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <iostream>
#include <catch2/catch.hpp>

namespace http  = boost::beast::http;
namespace range = boost::range;

TEST_CASE("Our detail::export_non_connect_fields function")
{
  SECTION("should preserve the Connection header fields and export the rest")
  {
    auto a = http::fields();
    auto b = http::fields();

    a.insert(http::field::connection, "rawr");
    a.insert(http::field::connection, "foo, bar, qux,,,     ,,lololol");
    a.insert("foo", "ha ha!");

    a.insert("nonconnectopt", "some random value");

    foxy::detail::export_non_connect_fields(a, b);

    range::for_each(
      a.equal_range(http::field::connection),
      [](auto const& field)
      {
        auto const value = field.value();
        std::cout << "value : " << value << "\n";
      });
  }
}
