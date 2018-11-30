//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_URI_DEF_HPP_
#define FOXY_DETAIL_URI_DEF_HPP_

#include <foxy/uri.hpp>

namespace foxy
{
namespace uri
{
namespace parser
{
namespace x3 = boost::spirit::x3;

x3::rule<class sub_delims> const sub_delims = "sub_delims";

auto const sub_delims_def = x3::lit("?");

BOOST_SPIRIT_DEFINE(sub_delims);

} // namespace parser

auto
sub_delims() -> parser::sub_delims_type
{
  return parser::sub_delims;
}

} // namespace uri
} // namespace foxy

#endif // FOXY_DETAIL_URI_DEF_HPP_
