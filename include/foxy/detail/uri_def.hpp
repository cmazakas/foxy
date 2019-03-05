//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_URI_DEF_HPP_
#define FOXY_DETAIL_URI_DEF_HPP_

#include <foxy/uri.hpp>

// A (hopefully) conforming implementation of the ABNF found at:
// https://tools.ietf.org/html/rfc3986#appendix-A
// written using Boost.Spirit.X3
//

namespace foxy
{
namespace uri
{
namespace parser
{
namespace x3 = boost::spirit::x3;

using char_set = x3::char_set<boost::spirit::char_encoding::ascii>;

// sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
//
x3::rule<class sub_delims> const sub_delims     = "sub_delims";
auto const                       sub_delims_def = char_set("!$&'()*+,;=");
BOOST_SPIRIT_DEFINE(sub_delims);

// gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
//
x3::rule<class gen_delims> const gen_delims     = "gen_delims";
auto const                       gen_delims_def = char_set(":/?#[]@");
BOOST_SPIRIT_DEFINE(gen_delims);

// reserved = gen-delims / sub-delims
//
x3::rule<class reserved> const reserved     = "reserved";
auto const                     reserved_def = gen_delims | sub_delims;
BOOST_SPIRIT_DEFINE(reserved);

// unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
//
x3::rule<class unreserved> const unreserved     = "unreserved";
auto const                       unreserved_def = x3::alpha | x3::digit | char_set("-._~");
BOOST_SPIRIT_DEFINE(unreserved);

// pct-encoded = "%" HEXDIG HEXDIG
//
x3::rule<class pct_encoded> const pct_encoded     = "pct_encoded";
auto const                        pct_encoded_def = x3::lit("%") >> x3::xdigit >> x3::xdigit;
BOOST_SPIRIT_DEFINE(pct_encoded);

// pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
//
x3::rule<class pchar> const pchar     = "pchar";
auto const                  pchar_def = unreserved | pct_encoded | sub_delims | char_set(":@");
BOOST_SPIRIT_DEFINE(pchar);

// query = *( pchar / "/" / "?" )
//
x3::rule<class query> const query     = "query";
auto const                  query_def = *(pchar | char_set("/?"));
BOOST_SPIRIT_DEFINE(query);

// fragment = *( pchar / "/" / "?" )
//
x3::rule<class fragment> const fragment     = "fragment";
auto const                     fragment_def = *(pchar | char_set("/?"));
BOOST_SPIRIT_DEFINE(fragment);

// segment = *pchar
//
x3::rule<class segment> const segment     = "segment";
auto const                    segment_def = *pchar;
BOOST_SPIRIT_DEFINE(segment);

// segment-nz = 1*pchar
//
x3::rule<class segment_nz> const segment_nz     = "segment_nz";
auto const                       segment_nz_def = +pchar;
BOOST_SPIRIT_DEFINE(segment_nz);

// segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//  ; non-zero-length segment without any colon ":"
//
x3::rule<class segment_nz_nc> const segment_nz_nc = "segment_nz_nc";
auto const segment_nz_nc_def                      = +(unreserved | pct_encoded | sub_delims | '@');
BOOST_SPIRIT_DEFINE(segment_nz_nc);

// path-empty = 0<pchar>
//
x3::rule<class path_empty> const path_empty     = "path_empty";
auto const                       path_empty_def = x3::repeat(0)[pchar];
BOOST_SPIRIT_DEFINE(path_empty);

// path-rootless = segment-nz *( "/" segment )
//
x3::rule<class path_rootless> const path_rootless     = "path_rootless";
auto const                          path_rootless_def = segment_nz >> *('/' >> segment);
BOOST_SPIRIT_DEFINE(path_rootless);

// path-noscheme = segment-nz-nc *( "/" segment )
//
x3::rule<class path_noscheme> const path_noscheme     = "path_noscheme";
auto const                          path_noscheme_def = segment_nz_nc >> *('/' >> segment);
BOOST_SPIRIT_DEFINE(path_noscheme);

// path-absolute = "/" [ segment-nz *( "/" segment ) ]
//
x3::rule<class path_absolute> const path_absolute = "path_absolute";
auto const path_absolute_def = x3::char_('/') >> -(segment_nz >> *('/' >> segment));
BOOST_SPIRIT_DEFINE(path_absolute);

// path-abempty = *( "/" segment )
//
x3::rule<class path_abempty> const path_abempty     = "path_abempty";
auto const                         path_abempty_def = *(x3::char_('/') >> segment);
BOOST_SPIRIT_DEFINE(path_abempty);

// path = path-abempty    ; begins with "/" or is empty
//      / path-absolute   ; begins with "/" but not "//"
//      / path-noscheme   ; begins with a non-colon segment
//      / path-rootless   ; begins with a segment
//      / path-empty      ; zero characters
//
x3::rule<class path> const path     = "path";
auto const                 path_def = path_abempty // begins with "/" or is empty
                      | path_absolute              // begins with "/" but not "//"
                      | path_noscheme              // begins with a non-colon segment
                      | path_rootless              // begins with a segment
                      | path_empty;                // zero characters
BOOST_SPIRIT_DEFINE(path);

// reg-name = *( unreserved / pct-encoded / sub-delims )
//
x3::rule<class reg_name> const reg_name     = "reg_name";
auto const                     reg_name_def = *(unreserved | pct_encoded | sub_delims);
BOOST_SPIRIT_DEFINE(reg_name);

//    dec-octet = DIGIT                 ; 0-9
//              / %x31-39 DIGIT         ; 10-99
//              / "1" 2DIGIT            ; 100-199
//              / "2" %x30-34 DIGIT     ; 200-249
//              / "25" %x30-35          ; 250-255
//
x3::rule<class dec_octet> const dec_octet = "dec_octet";
auto const                      dec_octet_def =
  (x3::lit("25") >> char_set("0-5")) | (x3::lit("2") >> char_set("0-4") >> x3::digit) |
  (x3::lit("1") >> x3::repeat(2)[x3::digit]) | (char_set("1-9") >> x3::digit) | x3::digit;
BOOST_SPIRIT_DEFINE(dec_octet);

// IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
//
x3::rule<class ip_v4_address> const ip_v4_address = "ip_v4_address";
auto const                          ip_v4_address_def =
  dec_octet >> "." >> dec_octet >> "." >> dec_octet >> "." >> dec_octet;
BOOST_SPIRIT_DEFINE(ip_v4_address);

// h16 = 1*4HEXDIG
//
x3::rule<class h16> const h16     = "h16";
auto const                h16_def = x3::repeat(1, 4)[x3::xdigit];
BOOST_SPIRIT_DEFINE(h16);

// ls32 = ( h16 ":" h16 ) / IPv4address
//
x3::rule<class ls32> const ls32     = "ls32";
auto const                 ls32_def = (h16 >> ":" >> h16) | ip_v4_address;
BOOST_SPIRIT_DEFINE(ls32);

// IPv6address   =
//
x3::rule<class ip_v6_address> const ip_v6_address = "ip_v6_address";
auto const                          ip_v6_address_def =
  // 6( h16 ":" ) ls32
  (x3::repeat(6)[h16 >> ":"] >> ls32) |

  // "::" 5( h16 ":" ) ls32
  (x3::lit("::") >> x3::repeat(5)[h16 >> ":"] >> ls32) |

  // [ h16 ] "::" 4( h16 ":" ) ls32
  (-h16 >> "::" >> x3::repeat(4)[h16 >> ":"] >> ls32) |

  // [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
  (-(x3::repeat(0, 1)[h16 >> ":"] >> h16) >> "::" >> x3::repeat(3)[h16 >> ":"] >> ls32) |

  // [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
  (-(h16 >> x3::repeat(0, 2)[x3::lit(":") >> h16]) >> "::" >> (ls32 >> x3::repeat(2)[":" >> h16])) |

  // [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32
  (-(h16 >> x3::repeat(0, 3)[":" >> h16]) >> "::" >> h16 >> ":" >> ls32) |

  // [ *4( h16 ":" ) h16 ] "::" ls32
  (-(h16 >> x3::repeat(0, 4)[x3::lit(":") >> h16]) >> "::" >> ls32) |

  // [ *5( h16 ":" ) h16 ] "::" h16
  (-(h16 >> x3::repeat(0, 5)[x3::lit(":") >> h16]) >> "::" >> h16) |

  // [ *6( h16 ":" ) h16 ] "::"
  (-(h16 >> x3::repeat(0, 6)[x3::lit(":") >> h16]) >> "::");
BOOST_SPIRIT_DEFINE(ip_v6_address);

// IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
//
x3::rule<class ip_vfuture> const ip_vfuture = "ip_vfuture";
auto const ip_vfuture_def = x3::lit("v") >> +x3::xdigit >> "." >> +(unreserved | sub_delims | ":");
BOOST_SPIRIT_DEFINE(ip_vfuture);

// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
//
x3::rule<class ip_literal> const ip_literal = "ip_literal";
auto const ip_literal_def                   = x3::lit("[") >> (ip_v6_address | ip_vfuture) >> "]";
BOOST_SPIRIT_DEFINE(ip_literal);

// port = *DIGIT
//
x3::rule<class port> const port     = "port";
auto const                 port_def = *x3::digit;
BOOST_SPIRIT_DEFINE(port);

// host = IP-literal / IPv4address / reg-name
//
x3::rule<class host> const host     = "host";
auto const                 host_def = ip_literal | ip_v4_address | reg_name;
BOOST_SPIRIT_DEFINE(host);

// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
//
x3::rule<class userinfo> const userinfo     = "userinfo";
auto const                     userinfo_def = *(unreserved | pct_encoded | sub_delims | ":");
BOOST_SPIRIT_DEFINE(userinfo);

// authority = [ userinfo "@" ] host [ ":" port ]
//
x3::rule<class authority> const authority     = "authority";
auto const                      authority_def = -(userinfo >> "@") >> host >> -(":" >> port);
BOOST_SPIRIT_DEFINE(authority);

// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
//
x3::rule<class scheme> const scheme     = "scheme";
auto const                   scheme_def = x3::alpha >> *(x3::alpha | x3::digit | char_set("+-."));
BOOST_SPIRIT_DEFINE(scheme);

// relative-part = "//" authority path-abempty
//               / path-absolute
//               / path-noscheme
//               / path-empty
//
x3::rule<class relative_part> const relative_part = "relative_part";
auto const                          relative_part_def =
  (x3::lit("//") >> authority >> path_abempty) | path_absolute | path_noscheme | path_empty;
BOOST_SPIRIT_DEFINE(relative_part);

// relative-ref = relative-part [ "?" query ] [ "#" fragment ]
//
x3::rule<class relative_ref> const relative_ref = "relative_ref";
auto const relative_ref_def = relative_part >> -("?" >> query) >> -("#" >> fragment);
BOOST_SPIRIT_DEFINE(relative_ref);

// hier-part = "//" authority path-abempty
//           / path-absolute
//           / path-rootless
//           / path-empty
//
x3::rule<class hier_part> const hier_part = "hier_part";
auto const                      hier_part_def =
  (x3::lit("//") >> authority >> path_abempty) | path_absolute | path_rootless | path_empty;
BOOST_SPIRIT_DEFINE(hier_part);

// URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
//
x3::rule<class uri> const uri = "uri";
auto const uri_def            = scheme >> ":" >> hier_part >> -("?" >> query) >> -("#" >> fragment);
BOOST_SPIRIT_DEFINE(uri);

// absolute-URI = scheme ":" hier-part [ "?" query ]
//
x3::rule<class absolute_uri> const absolute_uri     = "absolute_uri";
auto const                         absolute_uri_def = scheme >> ":" >> hier_part >> -("?" >> query);
BOOST_SPIRIT_DEFINE(absolute_uri);

// URI-reference = URI / relative-ref
//
x3::rule<class uri_reference> const uri_reference     = "uri_reference";
auto const                          uri_reference_def = uri | relative_ref;
BOOST_SPIRIT_DEFINE(uri_reference);

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

inline auto
ip_vfuture() -> parser::ip_vfuture_type
{
  return parser::ip_vfuture;
}

inline auto
ip_literal() -> parser::ip_literal_type
{
  return parser::ip_literal;
}

inline auto
port() -> parser::port_type
{
  return parser::port;
}

inline auto
host() -> parser::host_type
{
  return parser::host;
}

inline auto
userinfo() -> parser::userinfo_type
{
  return parser::userinfo;
}

inline auto
authority() -> parser::authority_type
{
  return parser::authority;
}

inline auto
scheme() -> parser::scheme_type
{
  return parser::scheme;
}

inline auto
relative_part() -> parser::relative_part_type
{
  return parser::relative_part;
}

inline auto
relative_ref() -> parser::relative_ref_type
{
  return parser::relative_ref;
}

inline auto
absolute_uri() -> parser::absolute_uri_type
{
  return parser::absolute_uri;
}

inline auto
uri_reference() -> parser::uri_reference_type
{
  return parser::uri_reference;
}

inline auto
hier_part() -> parser::hier_part_type
{
  return parser::hier_part;
}

inline auto
uri() -> parser::uri_type
{
  return parser::uri;
}

} // namespace uri
} // namespace foxy

#endif // FOXY_DETAIL_URI_DEF_HPP_
