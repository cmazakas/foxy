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
  x3::char_set<boost::spirit::char_encoding::ascii>("!$&'()*+,;=");
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
auto const pct_encoded_def = x3::lit("%") >> x3::xdigit >> x3::xdigit;
BOOST_SPIRIT_DEFINE(pct_encoded);

x3::rule<class pchar> const pchar     = "pchar";
auto const                  pchar_def = unreserved | pct_encoded | sub_delims |
                       x3::char_set<boost::spirit::char_encoding::ascii>(":@");
BOOST_SPIRIT_DEFINE(pchar);

x3::rule<class query> const query = "query";
auto const                  query_def =
  *pchar | x3::char_set<boost::spirit::char_encoding::ascii>("/?");
BOOST_SPIRIT_DEFINE(query);

x3::rule<class fragment> const fragment = "fragment";
auto const                     fragment_def =
  *pchar | x3::char_set<boost::spirit::char_encoding::ascii>("/?");
BOOST_SPIRIT_DEFINE(fragment);

x3::rule<class segment> const segment     = "segment";
auto const                    segment_def = *pchar;
BOOST_SPIRIT_DEFINE(segment);

x3::rule<class segment_nz> const segment_nz     = "segment_nz";
auto const                       segment_nz_def = +pchar;
BOOST_SPIRIT_DEFINE(segment_nz);

x3::rule<class segment_nz_nc> const segment_nz_nc = "segment_nz_nc";
auto const segment_nz_nc_def = +(unreserved | pct_encoded | sub_delims | "@");
BOOST_SPIRIT_DEFINE(segment_nz_nc);

x3::rule<class path_empty> const path_empty     = "path_empty";
auto const                       path_empty_def = x3::eps;
BOOST_SPIRIT_DEFINE(path_empty);

x3::rule<class path_rootless> const path_rootless = "path_rootless";
auto const path_rootless_def = segment_nz >> *("/" >> segment);
BOOST_SPIRIT_DEFINE(path_rootless);

x3::rule<class path_noscheme> const path_noscheme = "path_noscheme";
auto const path_noscheme_def = segment_nz_nc >> *("/" >> segment);
BOOST_SPIRIT_DEFINE(path_noscheme);

x3::rule<class path_absolute> const path_absolute     = "path_absolute";
auto const                          path_absolute_def = x3::lit("/") >>
                               -(segment_nz >> *("/" >> segment));
BOOST_SPIRIT_DEFINE(path_absolute);

x3::rule<class path_abempty> const path_abempty = "path_abempty";
auto const path_abempty_def                     = *(x3::lit("/") >> segment);
BOOST_SPIRIT_DEFINE(path_abempty);

x3::rule<class path> const path = "path";
auto const path_def             = path_abempty // begins with "/" or is empty
                      | path_absolute          // begins with "/" but not "//"
                      | path_noscheme // begins with a non-colon segment
                      | path_rootless // begins with a segment
                      | path_empty;   // zero characters
BOOST_SPIRIT_DEFINE(path);

x3::rule<class reg_name> const reg_name = "reg_name";
auto const reg_name_def = *(unreserved | pct_encoded | sub_delims);
BOOST_SPIRIT_DEFINE(reg_name);

x3::rule<class dec_octet> const dec_octet = "dec_octet";
auto const                      dec_octet_def =
  (x3::lit("25") >> x3::char_set<boost::spirit::char_encoding::ascii>("0-5")) |
  (x3::lit("2") >> x3::char_set<boost::spirit::char_encoding::ascii>("0-4") >>
   x3::digit) |
  (x3::lit("1") >> x3::repeat(2)[x3::digit]) |
  (x3::char_set<boost::spirit::char_encoding::ascii>("1-9") >> x3::digit) |
  x3::digit;
BOOST_SPIRIT_DEFINE(dec_octet);

x3::rule<class ip_v4_address> const ip_v4_address = "ip_v4_address";
auto const                          ip_v4_address_def =
  dec_octet >> "." >> dec_octet >> "." >> dec_octet >> "." >> dec_octet;
BOOST_SPIRIT_DEFINE(ip_v4_address);

x3::rule<class h16> const h16     = "h16";
auto const                h16_def = x3::repeat(1, 4)[x3::xdigit];
BOOST_SPIRIT_DEFINE(h16);

x3::rule<class ls32> const ls32     = "ls32";
auto const                 ls32_def = (h16 >> ":" >> h16) | ip_v4_address;
BOOST_SPIRIT_DEFINE(ls32);

x3::rule<class ip_v6_address> const ip_v6_address = "ip_v6_address";
auto const                          ip_v6_address_def =
  // 6( h16 ":" ) ls32
  (x3::repeat(6)[h16 >> ":"] >> ls32) |

  // "::" 5( h16 ":" ) ls32
  (x3::lit("::") >> x3::repeat(5)[h16 >> ":"] >> ls32) |

  //   [               h16 ] "::" 4( h16 ":" ) ls32
  (-h16 >> "::" >> x3::repeat(4)[h16 >> ":"] >> ls32) |

  // [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
  (-(x3::repeat(0, 1)[h16 >> ":"] >> h16) >> "::" >>
   x3::repeat(3)[h16 >> ":"] >> ls32) |

  // [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
  (-(h16 >> x3::repeat(0, 2)[x3::lit(":") >> h16]) >> "::" >>
   (ls32 >> x3::repeat(2)[":" >> h16])) |

  // [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
  (-(h16 >> x3::repeat(0, 3)[":" >> h16]) >> "::" >> h16 >> ":" >> ls32) |

  // [ *4( h16 ":" ) h16 ] "::"              ls32
  (-(h16 >> x3::repeat(0, 4)[x3::lit(":") >> h16]) >> "::" >> ls32) |

  // [ *5( h16 ":" ) h16 ] "::"              h16
  (-(h16 >> x3::repeat(0, 5)[x3::lit(":") >> h16]) >> "::" >> h16) |

  // [ *6( h16 ":" ) h16 ] "::"
  (-(h16 >> x3::repeat(0, 6)[x3::lit(":") >> h16]) >> "::");
BOOST_SPIRIT_DEFINE(ip_v6_address);

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

inline auto
query() -> parser::query_type
{
  return parser::query;
}

inline auto
fragment() -> parser::fragment_type
{
  return parser::fragment;
}

inline auto
segment() -> parser::segment_type
{
  return parser::segment;
}

inline auto
segment_nz() -> parser::segment_nz_type
{
  return parser::segment_nz;
}

inline auto
segment_nz_nc() -> parser::segment_nz_nc_type
{
  return parser::segment_nz_nc;
}

inline auto
path_empty() -> parser::path_empty_type
{
  return parser::path_empty;
}

inline auto
path_rootless() -> parser::path_rootless_type
{
  return parser::path_rootless;
}

inline auto
path_noscheme() -> parser::path_noscheme_type
{
  return parser::path_noscheme;
}

inline auto
path_absolute() -> parser::path_absolute_type
{
  return parser::path_absolute;
}

inline auto
path_abempty() -> parser::path_abempty_type
{
  return parser::path_abempty;
}

inline auto
path() -> parser::path_type
{
  return parser::path;
}

inline auto
reg_name() -> parser::reg_name_type
{
  return parser::reg_name;
}

inline auto
dec_octet() -> parser::dec_octet_type
{
  return parser::dec_octet;
}

inline auto
ip_v4_address() -> parser::ip_v4_address_type
{
  return parser::ip_v4_address;
}

inline auto
h16() -> parser::h16_type
{
  return parser::h16;
}

inline auto
ls32() -> parser::ls32_type
{
  return parser::ls32;
}

inline auto
ip_v6_address() -> parser::ip_v6_address_type
{
  return parser::ip_v6_address;
}

} // namespace uri
} // namespace foxy

#endif // FOXY_DETAIL_URI_DEF_HPP_
