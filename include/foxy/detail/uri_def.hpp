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
auto const                       sub_delims_def =
  +x3::char_set<boost::spirit::char_encoding::ascii>("!$&'()*+,;=");
BOOST_SPIRIT_DEFINE(sub_delims);

x3::rule<class gen_delims> const gen_delims = "gen_delims";
auto const                       gen_delims_def =
  x3::char_set<boost::spirit::char_encoding::ascii>(":/?#[]@");
BOOST_SPIRIT_DEFINE(gen_delims);

x3::rule<class reserved> const reserved     = "reserved";
auto const                     reserved_def = sub_delims | gen_delims;
BOOST_SPIRIT_DEFINE(reserved);

x3::rule<class unreserved> const unreserved = "unreserved";
auto const                       unreserved_def =
  x3::alpha | x3::digit |
  x3::char_set<boost::spirit::char_encoding::ascii>("-._~");
BOOST_SPIRIT_DEFINE(unreserved);

x3::rule<class pct_encoded> const pct_encoded = "pct_encoded";
auto const pct_encoded_def = x3::lit("%") > x3::xdigit > x3::xdigit;
BOOST_SPIRIT_DEFINE(pct_encoded);

x3::rule<class pchar> const pchar     = "pchar";
auto const                  pchar_def = unreserved | pct_encoded | sub_delims |
                       x3::char_set<boost::spirit::char_encoding::ascii>(":@");
BOOST_SPIRIT_DEFINE(pchar);

} // namespace parser

inline auto
sub_delims() -> parser::sub_delims_type
{
  return parser::sub_delims;
}

inline auto
gen_delims() -> parser::gen_delims_type
{
  return parser::gen_delims;
}

inline auto
reserved() -> parser::reserved_type
{
  return parser::reserved;
}

inline auto
unreserved() -> parser::unreserved_type
{
  return parser::unreserved;
}

inline auto
pct_encoded() -> parser::pct_encoded_type
{
  return parser::pct_encoded;
}

inline auto
pchar() -> parser::pchar_type
{
  return parser::pchar;
}

} // namespace uri
} // namespace foxy

#endif // FOXY_DETAIL_URI_DEF_HPP_
