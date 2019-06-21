//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_PCT_ENCODE_HPP_
#define FOXY_PCT_ENCODE_HPP_

#include <foxy/uri.hpp>
#include <foxy/code_point_iterator.hpp>

#include <boost/utility/string_view.hpp>
#include <boost/locale/utf.hpp>

#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_sequence.hpp>
#include <boost/spirit/include/karma_numeric.hpp>
#include <boost/spirit/include/karma_right_alignment.hpp>

#include <cstdint>
#include <cstring>
#include <string>
#include <array>
#include <iterator>
#include <type_traits>

namespace foxy
{
template <class OutputIterator>
auto
utf8_encode(char32_t const code_point, OutputIterator sink) -> OutputIterator
{
  return boost::locale::utf::utf_traits<char>::encode(code_point, sink);
}

// utf8_encode takes an interator pair to a range of char32_t's (or something convertible to a
// char32_t) and writes the utf-8 encoding to the output iterator denoted as `sink`
//
// https://en.wikipedia.org/wiki/UTF-8#Description
//
// UTF-8 encoding writes the representation of the code point's value to the binary templates
// described below:
//
// 7 bits =>  [U+0000,  U+007F]   => 0xxxxxxx
// 11 bits => [U+0080,  U+07FF]   => 110xxxxx 10xxxxxx
// 16 bits => [U+0800,  U+FFFF]   => 1110xxxx 10xxxxxx 10xxxxxx
// 21 bits => [U+10000, U+10FFFF] => 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
//
template <class InputIterator, class OutputIterator>
auto
utf8_encode(InputIterator begin, InputIterator end, OutputIterator sink) -> OutputIterator
{
  static_assert(
    std::is_convertible<typename std::iterator_traits<InputIterator>::value_type, char32_t>::value,
    "The InputIterator's value_type must be convertible to char32_t");

  for (auto curr = begin; curr != end; ++curr) {
    auto const code_point = *curr;

    sink = utf8_encode(code_point, sink);
  }

  return sink;
}

namespace uri
{
template <class OutputIterator>
auto
encode_host(boost::u32string_view const host, OutputIterator out) -> OutputIterator
{
  namespace x3    = boost::spirit::x3;
  namespace utf   = boost::locale::utf;
  namespace karma = boost::spirit::karma;

  auto pos = host.begin();

  auto match = x3::parse(pos, host.end(), unicode::ip_literal | unicode::ip_v4_address);
  if (match) {
    for (auto const code_point : host) { out = utf::utf_traits<char>::encode(code_point, out); }
    return out;
  }

  for (auto const code_point : host) {
    pos = host.begin();

    // no need to encode the normal ascii set
    //
    if ((code_point > 32) && (code_point < 127) &&
        boost::u32string_view(U"\"#/<>?@[\\]^`{|}").find(code_point) ==
          boost::u32string_view::npos) {
      out = utf::utf_traits<char>::encode(code_point, out);
      continue;
    }

    auto buffer = std::array<char, 4>{0x7f, 0x7f, 0x7f, 0x7f};

    auto const end = ::foxy::utf8_encode(code_point, buffer.begin());

    for (auto pos = buffer.begin(); pos < end; ++pos) {
      karma::generate(out, karma::lit("%") << karma::right_align(2, karma::lit("0"))[karma::hex],
                      *pos);
    }
  }

  return out;
}

template <class OutputIterator>
auto
encode_path(boost::u32string_view const host, OutputIterator out) -> OutputIterator
{
  namespace x3    = boost::spirit::x3;
  namespace utf   = boost::locale::utf;
  namespace karma = boost::spirit::karma;

  auto pos = host.begin();

  for (auto const code_point : host) {
    pos = host.begin();

    // no need to encode the normal ascii set
    //
    if ((code_point > 32) && (code_point < 127) &&
        boost::u32string_view(U"\"#<>?[\\]^`{|}").find(code_point) == boost::u32string_view::npos) {
      out = utf::utf_traits<char>::encode(code_point, out);
      continue;
    }

    auto buffer = std::array<char, 4>{0x7f, 0x7f, 0x7f, 0x7f};

    auto const end = ::foxy::utf8_encode(code_point, buffer.begin());

    for (auto pos = buffer.begin(); pos < end; ++pos) {
      karma::generate(out, karma::lit("%") << karma::right_align(2, karma::lit("0"))[karma::hex],
                      *pos);
    }
  }

  return out;
}

template <class OutputIterator>
auto
encode_query(boost::u32string_view const host, OutputIterator out) -> OutputIterator
{
  namespace x3    = boost::spirit::x3;
  namespace utf   = boost::locale::utf;
  namespace karma = boost::spirit::karma;

  auto pos = host.begin();

  for (auto const code_point : host) {
    pos = host.begin();

    // no need to encode the normal ascii set
    //
    if ((code_point > 32) && (code_point < 127) &&
        boost::u32string_view(U"\"#:<>@[\\]^`{|}").find(code_point) ==
          boost::u32string_view::npos) {
      out = utf::utf_traits<char>::encode(code_point, out);
      continue;
    }

    auto buffer = std::array<char, 4>{0x7f, 0x7f, 0x7f, 0x7f};

    auto const end = ::foxy::utf8_encode(code_point, buffer.begin());

    for (auto pos = buffer.begin(); pos < end; ++pos) {
      karma::generate(out, karma::lit("%") << karma::right_align(2, karma::lit("0"))[karma::hex],
                      *pos);
    }
  }

  return out;
}

template <class OutputIterator>
auto
encode_fragment(boost::u32string_view const host, OutputIterator out) -> OutputIterator
{
  return encode_query(host, out);
}

} // namespace uri
} // namespace foxy

#endif // FOXY_PCT_ENCODE_HPP_
