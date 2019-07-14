//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_UTF8_HPP_
#define FOXY_UTF8_HPP_

#include <boost/locale/utf.hpp>

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
} // namespace foxy

#endif // FOXY_UTF8_HPP_
