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

#include <boost/locale/utf.hpp>
#include <cstdint>

namespace foxy
{
namespace uri
{
auto
to_utf8_encoding(boost::locale::utf::code_point const code_point) -> std::uint32_t;

} // namespace uri
} // namespace foxy

#endif // FOXY_PCT_ENCODE_HPP_
