//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/pct_encode.hpp>

auto
foxy::uri::to_utf8_encoding(boost::locale::utf::code_point const code_point) -> std::uint32_t
{
  if (code_point < 0x80) { return code_point; }
  if (code_point < 0x0800) { return ((code_point & 0x7c0) << 2) + (code_point & 0x3f) + 0xc080; }
  return 0x0;
}
