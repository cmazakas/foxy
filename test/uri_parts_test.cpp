//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
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

    auto const uri_parts = foxy::parse_uri(view);

    CHECK(uri_parts.scheme() == "http");
    CHECK(uri_parts.host() == "www.google.com");
    CHECK(uri_parts.port() == "80");
    CHECK(uri_parts.path() == "/hello");
    CHECK(uri_parts.query() == "query");
    CHECK(uri_parts.fragment() == "fragment");

    CHECK(uri_parts.is_http());
    CHECK(!uri_parts.is_authority());
    CHECK(!uri_parts.is_absolute());
  }

  SECTION("should handle an empty URL")
  {
    auto const view = boost::string_view("http:");

    auto const uri_parts = foxy::parse_uri(view);

    CHECK(uri_parts.scheme() == "http");
    CHECK(uri_parts.host() == "");
    CHECK(uri_parts.port() == "");
    CHECK(uri_parts.path() == "");
    CHECK(uri_parts.query() == "");
    CHECK(uri_parts.fragment() == "");

    CHECK(uri_parts.is_http());
    CHECK(!uri_parts.is_authority());
    CHECK(uri_parts.is_absolute());
  }

  SECTION("should handle wonky user-input")
  {
    auto const view = boost::string_view("foof://:;@[::]/@;:??:;@/~@;://#//:;@~/@;:??//:foof");

    auto const uri_parts = foxy::parse_uri(view);

    CHECK(uri_parts.scheme() == "foof");
    CHECK(uri_parts.host() == "[::]");
    CHECK(uri_parts.port() == "");
    CHECK(uri_parts.path() == "/@;:");
    CHECK(uri_parts.query() == "?:;@/~@;://");
    CHECK(uri_parts.fragment() == "//:;@~/@;:??//:foof");

    CHECK(!uri_parts.is_http());
    CHECK(!uri_parts.is_authority());
    CHECK(!uri_parts.is_absolute());
  }

  SECTION("support a partial URI")
  {
    auto const view = boost::string_view("www.example.com:80");

    auto const uri_parts = foxy::parse_uri(view);

    CHECK(uri_parts.scheme() == "");
    CHECK(uri_parts.host() == "www.example.com");
    CHECK(uri_parts.port() == "80");
    CHECK(uri_parts.path() == "");
    CHECK(uri_parts.query() == "");
    CHECK(uri_parts.fragment() == "");

    CHECK(!uri_parts.is_http());
    CHECK(uri_parts.is_authority());
    CHECK(!uri_parts.is_absolute());
  }

  SECTION("support a partial URI wihout a port")
  {
    auto const view = boost::string_view("www.example.com/page1?user-info#lol");

    auto const uri_parts = foxy::parse_uri(view);

    CHECK(uri_parts.scheme() == "");
    CHECK(uri_parts.host() == "www.example.com");
    CHECK(uri_parts.port() == "");
    CHECK(uri_parts.path() == "/page1");
    CHECK(uri_parts.query() == "user-info");
    CHECK(uri_parts.fragment() == "lol");

    CHECK(!uri_parts.is_http());
    CHECK(uri_parts.is_authority());
    CHECK(!uri_parts.is_absolute());
  }

  SECTION("should handle another esoteric example")
  {
    auto const view = boost::string_view(
      "https://www.example.com/s/"
      "ref=sr_nr_p_n_feature_five_bro_0?fst=as%3Aoff&rh=n%3A172282%2Cn%3A!493964%2Cn%3A541966%2Cn%"
      "3A13896617011%2Cn%3A565108%2Cn%3A13896615011%2Cp_n_operating_system_browse-bin%"
      "3A12035945011%2Cp_n_intended_use_browse-bin%3A9647498011%2Cp_n_size_browse-bin%3A7817234011%"
      "2Cp_n_feature_five_browse-bin%3A13580790011%7c13580791011%7c13580788011%7c13580787011&bbn="
      "13896615011&ie=UTF8&qid=1484776142&rnid=2257851011");

    auto const uri_parts = foxy::parse_uri(view);

    CHECK(uri_parts.scheme() == "https");
    CHECK(uri_parts.host() == "www.example.com");
    CHECK(uri_parts.port() == "");
    CHECK(uri_parts.path() == "/s/ref=sr_nr_p_n_feature_five_bro_0");
    CHECK(
      uri_parts.query() ==
      "fst=as%3Aoff&rh=n%3A172282%2Cn%3A!493964%2Cn%3A541966%2Cn%"
      "3A13896617011%2Cn%3A565108%2Cn%3A13896615011%2Cp_n_operating_system_browse-bin%"
      "3A12035945011%2Cp_n_intended_use_browse-bin%3A9647498011%2Cp_n_size_browse-bin%3A7817234011%"
      "2Cp_n_feature_five_browse-bin%3A13580790011%7c13580791011%7c13580788011%7c13580787011&bbn="
      "13896615011&ie=UTF8&qid=1484776142&rnid=2257851011");
    CHECK(uri_parts.fragment() == "");

    CHECK(uri_parts.is_http());
    CHECK(!uri_parts.is_authority());
    CHECK(uri_parts.is_absolute());
  }

  SECTION("should handle yet another esoteric example")
  {
    auto const view = boost::string_view(
      "https://www.example.com/mn/search/tools/"
      "translate?ie=UTF8&page=1&rh=i%3Aaps%2Ck%3A1516700104%7C"
      "1516700112%7C1516700120%7C1516700139%7C1516700147%7C1516700155%7C1516700163%7C1630949019%"
      "7C1630949027%7C"
      "1630949043%7C1630949051%7C163094906X%7C1630949078%7C1630949086%7C1630949094%7C1630949108%"
      "7C1630949116%7C"
      "1630949124%7C1630949132%7C1630949140%7C1630949159%7C1630949167%7C1630949175%7C1630949183%"
      "7C1630949191%7C"
      "1630949205%7C1630949213%7C1630949221%7C163094923X%7C1630949248%7C1630949256%7C1630949264%"
      "7C1630949272%7C"
      "1630949280%7C1630949299%7C1630949302%7C1630949310%7C1630949329%7C1630949337%7C1630949426%"
      "7C1630949434%7C"
      "1630949442%7C1630949450%7C1630949469%7C1630949477%7C1630949485%7C1630949493%7C1630949507%"
      "7C1630949515%7C"
      "1630949523%7C1630949531%7C163094954X%7C1630949558%7C1630949566%7C1630949574%7C1630949582%"
      "7C1630949590%7C"
      "1630949604%7C1630949612%7C1630949620%7C1630949639%7C1630949647%7C1630949655%7C1630949663%"
      "7C1630949671%7C"
      "1630949035%7C163094968X%7C1630949698%7C1630949701%7C163094971X%7C1630949728%7C1630949736%"
      "2Cssx%3Arelevance");

    auto const uri_parts = foxy::parse_uri(view);

    CHECK(uri_parts.scheme() == "https");
    CHECK(uri_parts.host() == "www.example.com");
    CHECK(uri_parts.port() == "");
    CHECK(uri_parts.path() == "/mn/search/tools/translate");
    CHECK(
      uri_parts.query() ==
      "ie=UTF8&page=1&rh=i%3Aaps%2Ck%3A1516700104%7C"
      "1516700112%7C1516700120%7C1516700139%7C1516700147%7C1516700155%7C1516700163%7C1630949019%"
      "7C1630949027%7C"
      "1630949043%7C1630949051%7C163094906X%7C1630949078%7C1630949086%7C1630949094%7C1630949108%"
      "7C1630949116%7C"
      "1630949124%7C1630949132%7C1630949140%7C1630949159%7C1630949167%7C1630949175%7C1630949183%"
      "7C1630949191%7C"
      "1630949205%7C1630949213%7C1630949221%7C163094923X%7C1630949248%7C1630949256%7C1630949264%"
      "7C1630949272%7C"
      "1630949280%7C1630949299%7C1630949302%7C1630949310%7C1630949329%7C1630949337%7C1630949426%"
      "7C1630949434%7C"
      "1630949442%7C1630949450%7C1630949469%7C1630949477%7C1630949485%7C1630949493%7C1630949507%"
      "7C1630949515%7C"
      "1630949523%7C1630949531%7C163094954X%7C1630949558%7C1630949566%7C1630949574%7C1630949582%"
      "7C1630949590%7C"
      "1630949604%7C1630949612%7C1630949620%7C1630949639%7C1630949647%7C1630949655%7C1630949663%"
      "7C1630949671%7C"
      "1630949035%7C163094968X%7C1630949698%7C1630949701%7C163094971X%7C1630949728%7C1630949736%"
      "2Cssx%3Arelevance");
    CHECK(uri_parts.fragment() == "");

    CHECK(uri_parts.is_http());
    CHECK(!uri_parts.is_authority());
    CHECK(uri_parts.is_absolute());
  }
}
