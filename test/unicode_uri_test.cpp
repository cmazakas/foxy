//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/uri.hpp>
#include <boost/utility/string_view.hpp>
#include <vector>
#include <algorithm>

#include <catch2/catch.hpp>

namespace x3 = boost::spirit::x3;

TEST_CASE("unicode_uri_test")
{
  SECTION("should parse the sub-delims")
  {
    auto const delims = std::vector<boost::u32string_view>{U"!", U"$", U"&", U"'", U"(", U")",
                                                           U"*", U"+", U",", U";", U"="};

    auto const matched_all_sub_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::unicode::sub_delims);
      });

    CHECK(matched_all_sub_delims);

    auto       view  = boost::u32string_view(U"rawr");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const non_match = !x3::parse(begin, end, foxy::uri::unicode::sub_delims);

    CHECK(non_match);
  }

  SECTION("should parse the gen-delims")
  {
    auto const delims =
      std::vector<boost::u32string_view>{U":", U"/", U"?", U"#", U"[", U"]", U"@"};

    auto const matched_all_gen_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::unicode::gen_delims);
      });

    CHECK(matched_all_gen_delims);

    auto       view  = boost::u32string_view(U"rawr");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const non_match = !x3::parse(begin, end, foxy::uri::unicode::gen_delims);

    CHECK(non_match);
  }

  SECTION("should parse the reserved")
  {
    auto const delims =
      std::vector<boost::u32string_view>{U":", U"/", U"?", U"#", U"[", U"]", U"@", U"!", U"$",
                                         U"&", U"'", U"(", U")", U"*", U"+", U",", U";", U"="};

    auto const matched_all_reserved =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::unicode::reserved);
      });

    CHECK(matched_all_reserved);

    {
      auto       view  = boost::u32string_view(U"rawr");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const non_match = !x3::parse(begin, end, foxy::uri::unicode::reserved);

      CHECK(non_match);
    }

    {
      auto       view  = boost::u32string_view(U"~~~~Leonine.King1199__---");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match = x3::parse(begin, end, +foxy::uri::unicode::unreserved);

      CHECK(match);
      CHECK(begin == end);
    }
  }

  SECTION("should support percent encoded parsing")
  {
    auto       view  = boost::u32string_view(U"%5B");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const match = x3::parse(begin, end, foxy::uri::unicode::pct_encoded);

    CHECK(match);
    CHECK(begin == end);
  }

  SECTION("should support the pchar")
  {
    // unreserved + ":@" portion of pchar
    //
    {
      auto       view  = boost::u32string_view(U"~~:~~Le@on@ine.King1199__--:-");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match = x3::parse(begin, end, +foxy::uri::unicode::pchar);

      CHECK(match);
      CHECK(begin == end);
    }

    // pct_encoded portion of pchar
    //
    {
      auto       view  = boost::u32string_view(U"%5B");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match = x3::parse(begin, end, foxy::uri::unicode::pchar);

      CHECK(match);
      CHECK(begin == end);
    }

    // sub_delims portion of pchar
    //
    {
      auto const delims = std::vector<boost::u32string_view>{U"!", U"$", U"&", U"'", U"(", U")",
                                                             U"*", U"+", U",", U";", U"="};

      auto const matched_all_sub_delims =
        std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
          auto       begin = delim.begin();
          auto const end   = delim.end();

          return x3::parse(begin, end, foxy::uri::unicode::pchar);
        });

      CHECK(matched_all_sub_delims);
    }
  }

  SECTION("should support query/fragment parsing")
  {
    {
      auto       view  = boost::u32string_view(U"/lol?asdfasdfasdf");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match1 = x3::parse(begin, end, foxy::uri::unicode::query);

      begin             = view.begin();
      auto const match2 = x3::parse(begin, end, foxy::uri::unicode::fragment);

      CHECK((match1 && match2));
    }
  }

  SECTION("should support decimal octet parsing")
  {
    auto const valid_inputs = std::vector<boost::u32string_view>{
      U"0", U"1", U"9", U"10", U"99", U"100", U"199", U"200", U"249", U"250", U"255"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::unicode::dec_octet);

        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs =
      std::vector<boost::u32string_view>{U"lolol", U"-1", U"256", U"010", U"01", U"267", U"1337"};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::unicode::dec_octet);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }

  SECTION("should support IPv4 address parsing")
  {
    auto const valid_inputs = std::vector<boost::u32string_view>{U"127.0.0.1", U"255.255.255.255",
                                                                 U"0.0.0.0", U"192.68.0.27"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::unicode::ip_v4_address);

        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs = std::vector<boost::u32string_view>{
      U"127.0.0.01", U"255.255.255.255.255", U"a.b.c.d", U"192.68.334340.2227", U"127.0.1"};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::unicode::ip_v4_address);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }

  SECTION("should support IPv6 address parsing")
  {
    auto const valid_inputs =
      std::vector<boost::u32string_view>{U"3ffe:1900:4545:3:200:f8ff:fe21:67cf",
                                         U"fe80:0:0:0:200:f8ff:fe21:67cf",
                                         U"2001:0db8:0a0b:12f0:0000:0000:0000:0001",
                                         U"2001:db8:3333:4444:5555:6666:7777:8888",
                                         U"2001:db8:3333:4444:CCCC:DDDD:EEEE:FFFF",
                                         U"::",
                                         U"2001:db8::",
                                         U"::1234:5678",
                                         U"2001:db8::1234:5678",
                                         U"2001:0db8:0001:0000:0000:0ab9:C0A8:0102",
                                         U"2001:db8:1::ab9:C0A8:102",
                                         U"684D:1111:222:3333:4444:5555:6:77",
                                         U"0:0:0:0:0:0:0:0"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match      = x3::parse(begin, end, foxy::uri::unicode::ip_v6_address);
        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs = std::vector<boost::u32string_view>{};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::unicode::ip_v6_address);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }

  SECTION("should support URI parsing")
  {
    auto const valid_inputs =
      std::vector<boost::u32string_view>{U"https://www.google.com",
                                         U"http://example.com/",
                                         U"http://goo%20%20goo%7C%7C.com/",
                                         U"http://a.com/",
                                         U"http://192.168.0.1/",
                                         U"http://xn--6qqa088eba/",
                                         U"foobar://www.example.com:80/",
                                         U"http://example.com/foo%09%C2%91%93",
                                         U"http://example.com/%7Ffp3%3Eju%3Dduvgw%3Dd",
                                         U"http://www.example.com/?%02hello%7F%20bye",
                                         U"http://www.example.com/?q=%26%2355296%3B%26%2355296%3B",
                                         U"http://www.example.com/?foo=bar",
                                         U"http://www.example.com/#hello",
                                         U"http://www.example.com/#%23asdf",
                                         U"http:",
                                         U"asdf:jkl;",
                                         U"foof://:;@[::]/@;:??:;@/~@;://#//:;@~/@;:\?\?//:foof",
                                         U"http://ay%40lmao:password@[fe80::]/p@th?q=@lol",
                                         U"http://\u017C\u00F3\u0142\u0107.pl/"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match      = x3::parse(begin, end, foxy::uri::unicode::uri);
        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs =
      std::vector<boost::u32string_view>{U"http://192.168.0.1%20hello/", U"http://[google.com]/"};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::unicode::uri);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }
}
