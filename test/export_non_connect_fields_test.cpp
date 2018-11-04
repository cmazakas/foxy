//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/detail/export_non_connect_fields.hpp>
#include <boost/beast/http.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include <catch2/catch.hpp>

namespace http  = boost::beast::http;
namespace range = boost::range;

TEST_CASE("Our detail::export_non_connect_fields function")
{
  SECTION("should preserve the Connection header fields and export the rest")
  {
    auto const connect_tokens = { "rawr", "foo, bar, qux,,,     ,,lololol" };

    auto a = http::fields();
    auto b = http::fields();

    range::for_each(
      connect_tokens,
      [&](auto const* token) { a.insert(http::field::connection, token); });

    a.insert("foo", "ha ha!");
    a.insert("nonconnectopt", "some random value");

    foxy::detail::export_non_connect_fields(a, b);

    auto const still_has_connect_headers = range::equal(
      a.equal_range(http::field::connection),
      connect_tokens,
      [](auto const& field, auto const ex) -> bool
      {
        return field.value() == ex;
      });

    CHECK(still_has_connect_headers);

    auto const still_has_foo      = (a["foo"] == "ha ha!");
    auto const missing_nonconnect = (a["nonconnectopt"] == "");

    auto const has_nonconnect  = (b["nonconnectopt"] == "some random value");
    auto const missing_connect = (b[http::field::connection] == "");

    CHECK(still_has_foo);
    CHECK(missing_nonconnect);
    CHECK(has_nonconnect);
    CHECK(missing_connect);
  }
}
