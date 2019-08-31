//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/detail/export_connect_fields.hpp>

#include <boost/beast/http.hpp>

#include <boost/range/algorithm/equal.hpp>
#include <boost/range/algorithm/for_each.hpp>

#include <algorithm>

#include <catch2/catch.hpp>

namespace http  = boost::beast::http;
namespace range = boost::range;

TEST_CASE("export_connect_fields_test")
{
  SECTION("should export all hop-by-hops to an external Fields container")
  {
    auto const connect_tokens = {"rawr", "foo, bar, qux,,,     ,,lololol"};

    auto a = http::fields();
    auto b = http::fields();

    range::for_each(connect_tokens,
                    [&](auto const* token) { a.insert(http::field::connection, token); });

    a.insert(http::field::transfer_encoding, "gzip");
    a.insert(http::field::transfer_encoding, "chunked");

    // because "foo" appears in the Connection field, it's now a hop-by-hop
    //
    a.insert("foo", "ha ha!");

    // this is the one header that should remain in the fields
    //
    a.insert("nonconnectopt", "some random value");

    foxy::detail::export_connect_fields(a, b);

    auto const fields_range           = a.equal_range(http::field::connection);
    auto const missing_connect_fields = std::distance(fields_range.first, fields_range.second) == 0;

    CHECK(missing_connect_fields);

    auto const has_connect =
      range::equal(b.equal_range(http::field::connection), connect_tokens,
                   [](auto const& field, auto const ex) -> bool { return field.value() == ex; });

    CHECK(has_connect);

    CHECK(a["foo"] == "");
    CHECK(a["nonconnectopt"] == "some random value");
    CHECK(b["foo"] == "ha ha!");
    CHECK(b["nonconnectopt"] == "");
    CHECK(a[http::field::transfer_encoding] == "");

    auto transfer_encoding_iter = b.find(http::field::transfer_encoding);
    CHECK(transfer_encoding_iter->value() == "gzip");

    ++transfer_encoding_iter;
    CHECK(transfer_encoding_iter->value() == "chunked");
  }
}
