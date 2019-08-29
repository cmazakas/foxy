//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/pct_encode.hpp>
#include <boost/spirit/home/x3.hpp>

#include <string>
#include <cstdint>
#include <iterator>
#include <vector>
#include <bitset>
#include <array>

#include <catch2/catch.hpp>

namespace x3 = boost::spirit::x3;

TEST_CASE("pct_encode_test")
{
  SECTION(
    "... should be able to convert any Unicode code point to its underlying UTF-8 binary "
    "representation")
  {
    // [U+0000,  U+007F]
    //
    {
      auto const bytes_per_code_point = 1;
      auto const code_point_begin     = char32_t{0x0000};
      auto const code_point_end       = char32_t{0x0080};

      auto code_points = std::vector<char32_t>();
      code_points.reserve(code_point_end - code_point_begin);
      for (char32_t code_point = code_point_begin; code_point < code_point_end; ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<char>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::utf8_encode(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<8>(code_points[idx]);

        auto const first_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);

        is_valid_encoding = is_valid_encoding && !first_byte[7];
        is_valid_encoding = is_valid_encoding && (first_byte[6] == code_point[6]);
        is_valid_encoding = is_valid_encoding && (first_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (first_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (first_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }

    // [U+0080, U+0800]
    //
    {
      auto const bytes_per_code_point = 2;
      auto const code_point_begin     = char32_t{0x0080};
      auto const code_point_end       = char32_t{0x0800};

      auto code_points = std::vector<char32_t>();
      code_points.reserve(code_point_end - code_point_begin);
      for (char32_t code_point = code_point_begin; code_point < code_point_end; ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<char>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::utf8_encode(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<32>(code_points[idx]);

        auto const first_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);
        auto const second_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 1]);

        is_valid_encoding = is_valid_encoding && first_byte[7];
        is_valid_encoding = is_valid_encoding && first_byte[6];
        is_valid_encoding = is_valid_encoding && !first_byte[5];
        is_valid_encoding = is_valid_encoding && (first_byte[4] == code_point[10]);
        is_valid_encoding = is_valid_encoding && (first_byte[3] == code_point[9]);
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[8]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[7]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[6]);

        is_valid_encoding = is_valid_encoding && second_byte[7];
        is_valid_encoding = is_valid_encoding && !second_byte[6];
        is_valid_encoding = is_valid_encoding && (second_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (second_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (second_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (second_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (second_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (second_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }

    // [U+0800, U+10000]
    //
    {
      auto const bytes_per_code_point = 3;
      auto const code_point_begin     = char32_t{0x0800};
      auto const code_point_end       = char32_t{0x10000};

      auto code_points = std::vector<char32_t>();
      code_points.reserve(code_point_end - code_point_begin);
      for (char32_t code_point = code_point_begin; code_point < code_point_end; ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<char>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::utf8_encode(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<16>(code_points[idx]);

        auto const first_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);
        auto const second_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 1]);
        auto const third_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 2]);

        is_valid_encoding = is_valid_encoding && first_byte[7];
        is_valid_encoding = is_valid_encoding && first_byte[6];
        is_valid_encoding = is_valid_encoding && first_byte[5];
        is_valid_encoding = is_valid_encoding && !first_byte[4];
        is_valid_encoding = is_valid_encoding && (first_byte[3] == code_point[15]);
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[14]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[13]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[12]);

        is_valid_encoding = is_valid_encoding && second_byte[7];
        is_valid_encoding = is_valid_encoding && !second_byte[6];
        is_valid_encoding = is_valid_encoding && (second_byte[5] == code_point[11]);
        is_valid_encoding = is_valid_encoding && (second_byte[4] == code_point[10]);
        is_valid_encoding = is_valid_encoding && (second_byte[3] == code_point[9]);
        is_valid_encoding = is_valid_encoding && (second_byte[2] == code_point[8]);
        is_valid_encoding = is_valid_encoding && (second_byte[1] == code_point[7]);
        is_valid_encoding = is_valid_encoding && (second_byte[0] == code_point[6]);

        is_valid_encoding = is_valid_encoding && third_byte[7];
        is_valid_encoding = is_valid_encoding && !third_byte[6];
        is_valid_encoding = is_valid_encoding && (third_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (third_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (third_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (third_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (third_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (third_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }

    // [U+10000, U+110000]
    //
    {
      auto const bytes_per_code_point = 4;
      auto const code_point_begin     = char32_t{0x10000};
      auto const code_point_end       = char32_t{0x110000};

      auto code_points = std::vector<char32_t>();
      code_points.reserve(code_point_end - code_point_begin);
      for (char32_t code_point = code_point_begin; code_point < code_point_end; ++code_point) {
        code_points.push_back(code_point);
      }

      auto utf8_bytes = std::vector<char>();
      utf8_bytes.resize(bytes_per_code_point * code_points.size());

      foxy::utf8_encode(code_points.begin(), code_points.end(), utf8_bytes.begin());

      auto is_valid_encoding = true;

      for (std::size_t idx = 0; idx < code_points.size(); ++idx) {
        auto const code_point = std::bitset<21>(code_points[idx]);

        auto const first_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 0]);
        auto const second_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 1]);
        auto const third_byte  = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 2]);
        auto const fourth_byte = std::bitset<8>(utf8_bytes[(bytes_per_code_point * idx) + 3]);

        is_valid_encoding = is_valid_encoding && first_byte[7];
        is_valid_encoding = is_valid_encoding && first_byte[6];
        is_valid_encoding = is_valid_encoding && first_byte[5];
        is_valid_encoding = is_valid_encoding && first_byte[4];
        is_valid_encoding = is_valid_encoding && !first_byte[3];
        is_valid_encoding = is_valid_encoding && (first_byte[2] == code_point[20]);
        is_valid_encoding = is_valid_encoding && (first_byte[1] == code_point[19]);
        is_valid_encoding = is_valid_encoding && (first_byte[0] == code_point[18]);

        is_valid_encoding = is_valid_encoding && second_byte[7];
        is_valid_encoding = is_valid_encoding && !second_byte[6];
        is_valid_encoding = is_valid_encoding && (second_byte[5] == code_point[17]);
        is_valid_encoding = is_valid_encoding && (second_byte[4] == code_point[16]);
        is_valid_encoding = is_valid_encoding && (second_byte[3] == code_point[15]);
        is_valid_encoding = is_valid_encoding && (second_byte[2] == code_point[14]);
        is_valid_encoding = is_valid_encoding && (second_byte[1] == code_point[13]);
        is_valid_encoding = is_valid_encoding && (second_byte[0] == code_point[12]);

        is_valid_encoding = is_valid_encoding && third_byte[7];
        is_valid_encoding = is_valid_encoding && !third_byte[6];
        is_valid_encoding = is_valid_encoding && (third_byte[5] == code_point[11]);
        is_valid_encoding = is_valid_encoding && (third_byte[4] == code_point[10]);
        is_valid_encoding = is_valid_encoding && (third_byte[3] == code_point[9]);
        is_valid_encoding = is_valid_encoding && (third_byte[2] == code_point[8]);
        is_valid_encoding = is_valid_encoding && (third_byte[1] == code_point[7]);
        is_valid_encoding = is_valid_encoding && (third_byte[0] == code_point[6]);

        is_valid_encoding = is_valid_encoding && fourth_byte[7];
        is_valid_encoding = is_valid_encoding && !fourth_byte[6];
        is_valid_encoding = is_valid_encoding && (fourth_byte[5] == code_point[5]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[4] == code_point[4]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[3] == code_point[3]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[2] == code_point[2]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[1] == code_point[1]);
        is_valid_encoding = is_valid_encoding && (fourth_byte[0] == code_point[0]);
      }

      CHECK(is_valid_encoding);
    }
  }

  SECTION("[host] should not encode pct-encoded and the sub-delims")
  {
    auto const host = boost::u32string_view(U"%20%13%24!$'()*+,;=");

    auto out = std::string(256, '\0');

    auto const end          = foxy::uri::encode_host(host, out.begin());
    auto const encoded_host = boost::string_view(out.data(), end - out.begin());

    CHECK(encoded_host == "%20%13%24!$'()*+,;=");
  }

  SECTION("[host] should pct-encode misc. whitespace chars")
  {
    auto const host = boost::u32string_view(U"hello world!\n");
    auto       out  = std::string(256, '\0');

    auto const encoded_host =
      boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

    CHECK(encoded_host == "hello%20world!%0a");
  }

  SECTION("[host] should pct-encode a Polish hostname")
  {
    auto const host = boost::u32string_view(U"www.\u017C\u00F3\u0142\u0107.pl");
    auto       out  = std::string(256, '\0');

    auto const encoded_host =
      boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

    CHECK(encoded_host == "www.%c5%bc%c3%b3%c5%82%c4%87.pl");
  }

  SECTION("[host] should not pct-encode a valid IPv4 address")
  {
    auto const host = boost::u32string_view(U"127.0.0.1");
    auto       out  = std::string(256, '\0');

    auto const encoded_host =
      boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

    CHECK(encoded_host == "127.0.0.1");
  }

  SECTION("[host] should not pct-encode a valid IP-literal [IPv6 edition]")
  {
    auto const host = boost::u32string_view(U"[::]");
    auto       out  = std::string(256, '\0');

    auto const encoded_host =
      boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

    CHECK(encoded_host == "[::]");
  }

  SECTION("[host] should pct-encode seemingly random entries into the ASCII set")
  {
    auto const host = boost::u32string_view(U"\"#/<>?@[\\]^`{|}");
    auto       out  = std::string(256, '\0');

    auto const encoded_host =
      boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

    CHECK(encoded_host == "%22%23%2f%3c%3e%3f%40%5b%5c%5d%5e%60%7b%7c%7d");
  }

  SECTION("[host] should not pct-encode the ALPHA | DIGIT | -._~ | sub-delims")
  {
    auto const host = boost::u32string_view(
      U"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");

    auto out = std::string(256, '\0');

    auto const encoded_host =
      boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

    CHECK(encoded_host ==
          "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");
  }

  SECTION("[path] should not pct-encode normal paths")
  {
    auto const path = boost::u32string_view(
      U"/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");

    auto out = std::string(256, '\0');

    auto const encoded_path =
      boost::string_view(out.data(), foxy::uri::encode_path(path, out.begin()) - out.begin());

    CHECK(encoded_path ==
          "/ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=");
  }

  SECTION("[path] should not make decisions about pct-encoding the / separator")
  {
    auto const path = boost::u32string_view(U"////1.234/432/@@");

    auto out = std::string(256, '\0');

    auto const encoded_path =
      boost::string_view(out.data(), foxy::uri::encode_path(path, out.begin()) - out.begin());

    CHECK(encoded_path == "////1.234/432/@@");
  }

  SECTION("[path] should pct-encode odd characters in the path")
  {
    auto const path = boost::u32string_view(U"/\"#<>?@[\\]^`{|}:::");
    auto       out  = std::string(256, '\0');

    auto const encoded_path =
      boost::string_view(out.data(), foxy::uri::encode_path(path, out.begin()) - out.begin());

    CHECK(encoded_path == "/%22%23%3c%3e%3f@%5b%5c%5d%5e%60%7b%7c%7d:::");
  }

  SECTION("[path] shouldn't double-encode pct-encoded params in the path")
  {
    auto const path = boost::u32string_view(U"/%20%21%22%23");
    auto       out  = std::string(256, '\0');

    auto const encoded_path =
      boost::string_view(out.data(), foxy::uri::encode_path(path, out.begin()) - out.begin());

    CHECK(encoded_path == "/%20%21%22%23");
  }

  SECTION("[query] shouldn't double-encode pct-encoded params in the query")
  {
    auto const query = boost::u32string_view(U"?%20%21%22%23");
    auto       out   = std::string(256, '\0');

    auto const encoded_query =
      boost::string_view(out.data(), foxy::uri::encode_query(query, out.begin()) - out.begin());

    CHECK(encoded_query == "?%20%21%22%23");
  }

  SECTION("[query] shouldn't pct-encode the unreserved + sub-delims + /?")
  {
    auto const query = boost::u32string_view(
      U"?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=/?");
    auto out = std::string(256, '\0');

    auto const encoded_query =
      boost::string_view(out.data(), foxy::uri::encode_query(query, out.begin()) - out.begin());

    CHECK(encoded_query ==
          "?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~!$&'()*+,;=/?");
  }

  SECTION("[query] should pct-encode seemingly random entries into the ASCII set")
  {
    auto const query = boost::u32string_view(U"?//\":#/<>?@[\\]^`{|}");
    auto       out   = std::string(256, '\0');

    auto const encoded_query =
      boost::string_view(out.data(), foxy::uri::encode_query(query, out.begin()) - out.begin());

    CHECK(encoded_query == "?//%22%3a%23/%3c%3e?%40%5b%5c%5d%5e%60%7b%7c%7d");
  }

  SECTION("[query] should pct-encode Russian chars")
  {
    auto const query =
      boost::u32string_view(U"?q=\u0412\u0441\u0435\u043c \u043f\u0440\u0438\u0432\u0435\u0442");
    auto out = std::string(256, '\0');

    auto const encoded_query =
      boost::string_view(out.data(), foxy::uri::encode_query(query, out.begin()) - out.begin());

    CHECK(encoded_query == "?q=%d0%92%d1%81%d0%b5%d0%bc%20%d0%bf%d1%80%d0%b8%d0%b2%d0%b5%d1%82");
  }
}
