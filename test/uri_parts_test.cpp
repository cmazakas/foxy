//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/uri_parts.hpp>
#include <boost/utility/string_view.hpp>

#include <catch2/catch.hpp>

TEST_CASE("Our uri_parts function")
{
  SECTION("should be able to decompose a well-formed URI")
  {
    auto const view = boost::string_view("http://www.google.com:80/hello?query#fragment");

    auto const uri_parts = foxy::make_uri_parts(view);

    CHECK(uri_parts.scheme() == "http");
    CHECK(uri_parts.host() == "www.google.com");
    CHECK(uri_parts.port() == "80");
    CHECK(uri_parts.path() == "/hello");
    CHECK(uri_parts.query() == "query");
    CHECK(uri_parts.fragment() == "fragment");
    CHECK(uri_parts.is_http());
    CHECK(uri_parts.is_authority());
  }

  SECTION("should handle an empty URL")
  {
    auto const view = boost::string_view("http:");

    auto const uri_parts = foxy::make_uri_parts(view);

    CHECK(uri_parts.scheme() == "http");
    CHECK(uri_parts.host() == "");
    CHECK(uri_parts.port() == "");
    CHECK(uri_parts.path() == "");
    CHECK(uri_parts.query() == "");
    CHECK(uri_parts.fragment() == "");
    CHECK(uri_parts.is_http());
    CHECK(!uri_parts.is_authority());
  }

  SECTION("should handle wonky user-input")
  {
    auto const view = boost::string_view("foof://:;@[::]/@;:??:;@/~@;://#//:;@~/@;:??//:foof");

    auto const uri_parts = foxy::make_uri_parts(view);

    CHECK(uri_parts.scheme() == "foof");
    CHECK(uri_parts.host() == "[::]");
    CHECK(uri_parts.port() == "");
    CHECK(uri_parts.path() == "/@;:");
    CHECK(uri_parts.query() == "?:;@/~@;://");
    CHECK(uri_parts.fragment() == "//:;@~/@;:??//:foof");
    CHECK(!uri_parts.is_http());
    CHECK(uri_parts.is_authority());
  }
}
