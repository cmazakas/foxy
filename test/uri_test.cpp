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

TEST_CASE("uri_test")
{
  SECTION("should parse the sub-delims")
  {
    auto const delims =
      std::vector<boost::string_view>{"!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="};

    auto const matched_all_sub_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::sub_delims);
      });

    CHECK(matched_all_sub_delims);

    auto       view  = boost::string_view("rawr");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const non_match = !x3::parse(begin, end, foxy::uri::sub_delims);

    CHECK(non_match);
  }

  SECTION("should parse the gen-delims")
  {
    auto const delims = std::vector<boost::string_view>{":", "/", "?", "#", "[", "]", "@"};

    auto const matched_all_gen_delims =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::gen_delims);
      });

    CHECK(matched_all_gen_delims);

    auto       view  = boost::string_view("rawr");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const non_match = !x3::parse(begin, end, foxy::uri::gen_delims);

    CHECK(non_match);
  }

  SECTION("should parse the reserved")
  {
    auto const delims = std::vector<boost::string_view>{
      ":", "/", "?", "#", "[", "]", "@", "!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="};

    auto const matched_all_reserved =
      std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
        auto       begin = delim.begin();
        auto const end   = delim.end();

        return x3::parse(begin, end, foxy::uri::reserved);
      });

    CHECK(matched_all_reserved);

    {
      auto       view  = boost::string_view("rawr");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const non_match = !x3::parse(begin, end, foxy::uri::reserved);

      CHECK(non_match);
    }

    {
      auto       view  = boost::string_view("~~~~Leonine.King1199__---");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match = x3::parse(begin, end, +foxy::uri::unreserved);

      CHECK(match);
      CHECK(begin == end);
    }
  }

  SECTION("should support percent encoded parsing")
  {
    auto       view  = boost::string_view("%5B");
    auto       begin = view.begin();
    auto const end   = view.end();

    auto const match = x3::parse(begin, end, foxy::uri::pct_encoded);

    CHECK(match);
    CHECK(begin == end);
  }

  SECTION("should support the pchar")
  {
    // unreserved + ":@" portion of pchar
    //
    {
      auto       view  = boost::string_view("~~:~~Le@on@ine.King1199__--:-");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match = x3::parse(begin, end, +foxy::uri::pchar);

      CHECK(match);
      CHECK(begin == end);
    }

    // pct_encoded portion of pchar
    //
    {
      auto       view  = boost::string_view("%5B");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match = x3::parse(begin, end, foxy::uri::pchar);

      CHECK(match);
      CHECK(begin == end);
    }

    // sub_delims portion of pchar
    //
    {
      auto const delims =
        std::vector<boost::string_view>{"!", "$", "&", "'", "(", ")", "*", "+", ",", ";", "="};

      auto const matched_all_sub_delims =
        std::all_of(delims.begin(), delims.end(), [](auto const delim) -> bool {
          auto       begin = delim.begin();
          auto const end   = delim.end();

          return x3::parse(begin, end, foxy::uri::pchar);
        });

      CHECK(matched_all_sub_delims);
    }
  }

  SECTION("should support query/fragment parsing")
  {
    {
      auto       view  = boost::string_view("/lol?asdfasdfasdf");
      auto       begin = view.begin();
      auto const end   = view.end();

      auto const match1 = x3::parse(begin, end, foxy::uri::query);

      begin             = view.begin();
      auto const match2 = x3::parse(begin, end, foxy::uri::fragment);

      CHECK((match1 && match2));
    }
  }

  SECTION("should support decimal octet parsing")
  {
    auto const valid_inputs = std::vector<boost::string_view>{
      "0", "1", "9", "10", "99", "100", "199", "200", "249", "250", "255"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::dec_octet);

        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs =
      std::vector<boost::string_view>{"lolol", "-1", "256", "010", "01", "267", "1337"};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::dec_octet);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }

  SECTION("should support IPv4 address parsing")
  {
    auto const valid_inputs =
      std::vector<boost::string_view>{"127.0.0.1", "255.255.255.255", "0.0.0.0", "192.68.0.27"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::ip_v4_address);

        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs = std::vector<boost::string_view>{
      "127.0.0.01", "255.255.255.255.255", "a.b.c.d", "192.68.334340.2227", "127.0.1"};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::ip_v4_address);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }

  SECTION("should support IPv6 address parsing")
  {
    auto const valid_inputs =
      std::vector<boost::string_view>{"3ffe:1900:4545:3:200:f8ff:fe21:67cf",
                                      "fe80:0:0:0:200:f8ff:fe21:67cf",
                                      "2001:0db8:0a0b:12f0:0000:0000:0000:0001",
                                      "2001:db8:3333:4444:5555:6666:7777:8888",
                                      "2001:db8:3333:4444:CCCC:DDDD:EEEE:FFFF",
                                      "::",
                                      "2001:db8::",
                                      "::1234:5678",
                                      "2001:db8::1234:5678",
                                      "2001:0db8:0001:0000:0000:0ab9:C0A8:0102",
                                      "2001:db8:1::ab9:C0A8:102",
                                      "684D:1111:222:3333:4444:5555:6:77",
                                      "0:0:0:0:0:0:0:0"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match      = x3::parse(begin, end, foxy::uri::ip_v6_address);
        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs = std::vector<boost::string_view>{};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::ip_v6_address);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }

  SECTION("should support URI parsing")
  {
    auto const valid_inputs =
      std::vector<boost::string_view>{"https://www.google.com",
                                      "http://example.com/",
                                      "http://goo%20%20goo%7C%7C.com/",
                                      "http://a.com/",
                                      "http://192.168.0.1/",
                                      "http://xn--6qqa088eba/",
                                      "foobar://www.example.com:80/",
                                      "http://example.com/foo%09%C2%91%93",
                                      "http://example.com/%7Ffp3%3Eju%3Dduvgw%3Dd",
                                      "http://www.example.com/?%02hello%7F%20bye",
                                      "http://www.example.com/?q=%26%2355296%3B%26%2355296%3B",
                                      "http://www.example.com/?foo=bar",
                                      "http://www.example.com/#hello",
                                      "http://www.example.com/#%23asdf",
                                      "http:",
                                      "asdf:jkl;",
                                      "foof://:;@[::]/@;:??:;@/~@;://#//:;@~/@;:\?\?//:foof",
                                      "http://ay%40lmao:password@[fe80::]/p@th?q=@lol"};

    auto const all_match =
      std::all_of(valid_inputs.begin(), valid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match      = x3::parse(begin, end, foxy::uri::uri);
        auto const full_match = match && (begin == end);

        return full_match;
      });

    CHECK(all_match);

    auto const invalid_inputs =
      std::vector<boost::string_view>{"http://192.168.0.1%20hello/", "http://[google.com]/"};

    auto const none_match =
      std::all_of(invalid_inputs.begin(), invalid_inputs.end(), [](auto const view) -> bool {
        auto       begin = view.begin();
        auto const end   = view.end();

        auto const match = x3::parse(begin, end, foxy::uri::uri);

        if (match) { return begin != end; }
        return !match;
      });

    CHECK(none_match);
  }
}
