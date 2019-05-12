//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_URI_HPP_
#define FOXY_URI_HPP_

#define BOOST_SPIRIT_X3_UNICODE

#include <boost/spirit/home/x3.hpp>
#include <boost/utility/string_view.hpp>

// all credit for the port of `foxy::uri::raw` goes to Kojoley from github:
// https://github.com/boostorg/spirit/issues/487#issuecomment-481899288
//
// without them, this wouldn't have been at all possible for the library
//
namespace boost
{
namespace spirit
{
namespace x3
{
namespace traits
{
template <typename Char, typename Trait>
struct is_range<boost::basic_string_view<Char, Trait>> : boost::mpl::true_
{
};
} // namespace traits
} // namespace x3
} // namespace spirit
} // namespace boost

namespace foxy
{
// A (hopefully) conforming implementation of the ABNF found at:
// https://tools.ietf.org/html/rfc3986#appendix-A
//
// written using Boost.Spirit.X3
//

namespace uri
{
namespace x3 = boost::spirit::x3;

template <class Subject>
struct raw_directive : x3::raw_directive<Subject>
{
  using x3::raw_directive<Subject>::raw_directive;

  template <typename Iterator, typename Context, typename RContext, typename Attribute>
  bool
  parse(Iterator&       first,
        Iterator const& last,
        Context const&  context,
        RContext&       rcontext,
        Attribute&      attr) const
  {
    x3::skip_over(first, last, context);
    Iterator saved = first;
    if (this->subject.parse(first, last, context, rcontext, x3::unused)) {
      attr = {saved, typename Attribute::size_type(first - saved)};
      return true;
    }
    return false;
  }
};

struct raw_gen
{
  template <class Subject>
  raw_directive<typename x3::extension::as_parser<Subject>::value_type>
  operator[](Subject subject) const
  {
    return {x3::as_parser(std::move(subject))};
  }
};

auto const raw = raw_gen{};

// sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
//
x3::rule<class sub_delims> const sub_delims     = "sub_delims";
auto const                       sub_delims_def = x3::char_("!$&'()*+,;=");
BOOST_SPIRIT_DEFINE(sub_delims);

// gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
//
x3::rule<class gen_delims> const gen_delims     = "gen_delims";
auto const                       gen_delims_def = x3::char_(":/?#[]@");
BOOST_SPIRIT_DEFINE(gen_delims);

// reserved = gen-delims / sub-delims
//
x3::rule<class reserved> const reserved     = "reserved";
auto const                     reserved_def = gen_delims | sub_delims;
BOOST_SPIRIT_DEFINE(reserved);

// unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
//
x3::rule<class unreserved> const unreserved     = "unreserved";
auto const                       unreserved_def = x3::alpha | x3::digit | x3::char_("-._~");
BOOST_SPIRIT_DEFINE(unreserved);

// pct-encoded = "%" HEXDIG HEXDIG
//
x3::rule<class pct_encoded> const pct_encoded     = "pct_encoded";
auto const                        pct_encoded_def = x3::lit("%") >> x3::xdigit >> x3::xdigit;
BOOST_SPIRIT_DEFINE(pct_encoded);

// pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
//
x3::rule<class pchar> const pchar     = "pchar";
auto const                  pchar_def = unreserved | pct_encoded | sub_delims | x3::char_(":@");
BOOST_SPIRIT_DEFINE(pchar);

// query = *( pchar / "/" / "?" )
//
x3::rule<class query> const query     = "query";
auto const                  query_def = *(pchar | x3::char_("/?"));
BOOST_SPIRIT_DEFINE(query);

// fragment = *( pchar / "/" / "?" )
//
x3::rule<class fragment> const fragment     = "fragment";
auto const                     fragment_def = *(pchar | x3::char_("/?"));
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
  (x3::lit("25") >> x3::char_("0-5")) | (x3::lit("2") >> x3::char_("0-4") >> x3::digit) |
  (x3::lit("1") >> x3::repeat(2)[x3::digit]) | (x3::char_("1-9") >> x3::digit) | x3::digit;
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
auto const                   scheme_def = x3::alpha >> *(x3::alpha | x3::digit | x3::char_("+-."));
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

// A Unicode port of the above PEGs
// Works on Unicode code point literals which in code is siply char32_t
//
namespace unicode
{
// sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
//
x3::rule<class unicode_sub_delims> const sub_delims     = "unicode_sub_delims";
auto const                               sub_delims_def = x3::unicode::char_(U"!$&'()*+,;=");
BOOST_SPIRIT_DEFINE(sub_delims);

// gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
//
x3::rule<class unicode_gen_delims> const gen_delims     = "unicode_gen_delims";
auto const                               gen_delims_def = x3::unicode::char_(U":/?#[]@");
BOOST_SPIRIT_DEFINE(gen_delims);

// reserved = gen-delims / sub-delims
//
x3::rule<class unicode_reserved> const reserved     = "unicode_reserved";
auto const                             reserved_def = gen_delims | sub_delims;
BOOST_SPIRIT_DEFINE(reserved);

// unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
//
x3::rule<class unicode_unreserved> const unreserved = "unicode_unreserved";
auto const unreserved_def = x3::unicode::alpha | x3::unicode::digit | x3::unicode::char_(U"-._~");
BOOST_SPIRIT_DEFINE(unreserved);

// pct-encoded = "%" HEXDIG HEXDIG
//
x3::rule<class unicode_pct_encoded> const pct_encoded = "unicode_pct_encoded";
auto const pct_encoded_def = x3::unicode::lit(U"%") >> x3::unicode::xdigit >> x3::unicode::xdigit;
BOOST_SPIRIT_DEFINE(pct_encoded);

// pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
//
x3::rule<class unicode_pchar> const pchar = "unicode_pchar";
auto const pchar_def = unreserved | pct_encoded | sub_delims | x3::unicode::char_(U":@");
BOOST_SPIRIT_DEFINE(pchar);

// query = *( pchar / "/" / "?" )
//
x3::rule<class unicode_query> const query     = "unicode_query";
auto const                          query_def = *(pchar | x3::unicode::char_(U"/?"));
BOOST_SPIRIT_DEFINE(query);

// fragment = *( pchar / "/" / "?" )
//
x3::rule<class unicode_fragment> const fragment     = "unicode_fragment";
auto const                             fragment_def = *(pchar | x3::unicode::char_(U"/?"));
BOOST_SPIRIT_DEFINE(fragment);

// segment = *pchar
//
x3::rule<class unicode_segment> const segment     = "unicode_segment";
auto const                            segment_def = *pchar;
BOOST_SPIRIT_DEFINE(segment);

// segment-nz = 1*pchar
//
x3::rule<class unicode_segment_nz> const segment_nz     = "unicode_segment_nz";
auto const                               segment_nz_def = +pchar;
BOOST_SPIRIT_DEFINE(segment_nz);

// segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//  ; non-zero-length segment without any colon ":"
//
x3::rule<class unicode_segment_nz_nc> const segment_nz_nc = "unicode_segment_nz_nc";
auto const segment_nz_nc_def = +(unreserved | pct_encoded | sub_delims | x3::unicode::char_(U'@'));
BOOST_SPIRIT_DEFINE(segment_nz_nc);

// path-empty = 0<pchar>
//
x3::rule<class unicode_path_empty> const path_empty     = "unicode_path_empty";
auto const                               path_empty_def = x3::repeat(0)[pchar];
BOOST_SPIRIT_DEFINE(path_empty);

// path-rootless = segment-nz *( "/" segment )
//
x3::rule<class unicode_path_rootless> const path_rootless = "unicode_path_rootless";
auto const path_rootless_def = segment_nz >> *(x3::unicode::char_(U'/') >> segment);
BOOST_SPIRIT_DEFINE(path_rootless);

// path-noscheme = segment-nz-nc *( "/" segment )
//
x3::rule<class unicode_path_noscheme> const path_noscheme = "unicode_path_noscheme";
auto const path_noscheme_def = segment_nz_nc >> *(x3::unicode::char_(U'/') >> segment);
BOOST_SPIRIT_DEFINE(path_noscheme);

// path-absolute = "/" [ segment-nz *( "/" segment ) ]
//
x3::rule<class unicode_path_absolute> const path_absolute     = "unicode_path_absolute";
auto const                                  path_absolute_def = x3::unicode::char_(U'/') >>
                               -(segment_nz >> *(x3::unicode::char_(U'/') >> segment));
BOOST_SPIRIT_DEFINE(path_absolute);

// path-abempty = *( "/" segment )
//
x3::rule<class unicode_path_abempty> const path_abempty = "unicode_path_abempty";
auto const path_abempty_def                             = *(x3::unicode::char_(U'/') >> segment);
BOOST_SPIRIT_DEFINE(path_abempty);

// path = path-abempty    ; begins with "/" or is empty
//      / path-absolute   ; begins with "/" but not "//"
//      / path-noscheme   ; begins with a non-colon segment
//      / path-rootless   ; begins with a segment
//      / path-empty      ; zero characters
//
x3::rule<class unicode_path> const path     = "unicode_path";
auto const                         path_def = path_abempty // begins with "/" or is empty
                      | path_absolute                      // begins with "/" but not "//"
                      | path_noscheme                      // begins with a non-colon segment
                      | path_rootless                      // begins with a segment
                      | path_empty;                        // zero characters
BOOST_SPIRIT_DEFINE(path);

// reg-name = *( unreserved / pct-encoded / sub-delims )
//
x3::rule<class unicode_reg_name> const reg_name     = "unicode_reg_name";
auto const                             reg_name_def = *(unreserved | pct_encoded | sub_delims);
BOOST_SPIRIT_DEFINE(reg_name);

//    dec-octet = DIGIT                 ; 0-9
//              / %x31-39 DIGIT         ; 10-99
//              / "1" 2DIGIT            ; 100-199
//              / "2" %x30-34 DIGIT     ; 200-249
//              / "25" %x30-35          ; 250-255
//
x3::rule<class unicode_dec_octet> const dec_octet = "unicode_dec_octet";
auto const                              dec_octet_def =
  (x3::unicode::lit(U"25") >> x3::unicode::char_(U"0-5")) |
  (x3::unicode::lit(U"2") >> x3::unicode::char_(U"0-4") >> x3::unicode::digit) |
  (x3::unicode::lit(U"1") >> x3::repeat(2)[x3::unicode::digit]) |
  (x3::unicode::char_("1-9") >> x3::unicode::digit) | x3::unicode::digit;
BOOST_SPIRIT_DEFINE(dec_octet);

// IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
//
x3::rule<class unicode_ip_v4_address> const ip_v4_address = "unicode_ip_v4_address";
auto const ip_v4_address_def = dec_octet >> x3::unicode::lit(U".") >> dec_octet
                               >> x3::unicode::lit(U".") >> dec_octet
                               >> x3::unicode::lit(U".") >> dec_octet;
BOOST_SPIRIT_DEFINE(ip_v4_address);

// h16 = 1*4HEXDIG
//
x3::rule<class unicode_h16> const h16     = "unicode_h16";
auto const                        h16_def = x3::repeat(1, 4)[x3::unicode::xdigit];
BOOST_SPIRIT_DEFINE(h16);

// ls32 = ( h16 ":" h16 ) / IPv4address
//
x3::rule<class unicode_ls32> const ls32 = "unicode_ls32";
auto const ls32_def                     = (h16 >> x3::unicode::lit(U":") >> h16) | ip_v4_address;
BOOST_SPIRIT_DEFINE(ls32);

// IPv6address   =
//
x3::rule<class unicode_ip_v6_address> const ip_v6_address = "unicode_ip_v6_address";
auto const                                  ip_v6_address_def =
  // 6( h16 ":" ) ls32
  (x3::repeat(6)[h16 >> x3::unicode::lit(U":")] >> ls32) |

  // "::" 5( h16 ":" ) ls32
  (x3::unicode::lit(U"::") >> x3::repeat(5)[h16 >> x3::unicode::lit(U":")] >> ls32) |

  // [ h16 ] "::" 4( h16 ":" ) ls32
  (-h16 >> x3::unicode::lit(U"::") >> x3::repeat(4)[h16 >> x3::unicode::lit(U":")] >> ls32) |

  // [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
  (-(x3::repeat(0, 1)[h16 >> x3::unicode::lit(U":")] >> h16) >> x3::unicode::lit(U"::") >>
   x3::repeat(3)[h16 >> x3::unicode::lit(U":")] >> ls32) |

  // [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
  (-(h16 >> x3::repeat(0, 2)[x3::unicode::lit(U":") >> h16]) >> x3::unicode::lit(U"::") >>
   (ls32 >> x3::repeat(2)[x3::unicode::lit(U":") >> h16])) |

  // [ *3( h16 ":" ) h16 ] "::" h16 ":" ls32
  (-(h16 >> x3::repeat(0, 3)[x3::unicode::lit(U":") >> h16]) >> x3::unicode::lit(U"::") >> h16 >>
   x3::unicode::lit(U":") >> ls32) |

  // [ *4( h16 ":" ) h16 ] "::" ls32
  (-(h16 >> x3::repeat(0, 4)[x3::unicode::lit(U":") >> h16]) >> x3::unicode::lit(U"::") >> ls32) |

  // [ *5( h16 ":" ) h16 ] "::" h16
  (-(h16 >> x3::repeat(0, 5)[x3::unicode::lit(U":") >> h16]) >> x3::unicode::lit(U"::") >> h16) |

  // [ *6( h16 ":" ) h16 ] "::"
  (-(h16 >> x3::repeat(0, 6)[x3::unicode::lit(U":") >> h16]) >> x3::unicode::lit(U"::"));
BOOST_SPIRIT_DEFINE(ip_v6_address);

// IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
//
x3::rule<class unicode_ip_vfuture> const ip_vfuture = "unicode_ip_vfuture";
auto const ip_vfuture_def = x3::unicode::lit(U"v") >> +x3::unicode::xdigit >>
                            x3::unicode::lit(U".") >>
                            +(unreserved | sub_delims | x3::unicode::lit(U":"));
BOOST_SPIRIT_DEFINE(ip_vfuture);

// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
//
x3::rule<class unicode_ip_literal> const ip_literal     = "unicode_ip_literal";
auto const                               ip_literal_def = x3::unicode::lit(U"[") >>
                            (ip_v6_address | ip_vfuture) >> x3::unicode::lit(U"]");
BOOST_SPIRIT_DEFINE(ip_literal);

// port = *DIGIT
//
x3::rule<class unicode_port> const port     = "unicode_port";
auto const                         port_def = *x3::unicode::digit;
BOOST_SPIRIT_DEFINE(port);

// host = IP-literal / IPv4address / reg-name
//
x3::rule<class unicode_host> const host     = "unicode_host";
auto const                         host_def = ip_literal | ip_v4_address | reg_name;
BOOST_SPIRIT_DEFINE(host);

// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
//
x3::rule<class unicode_userinfo> const userinfo = "unicode_userinfo";
auto const userinfo_def = *(unreserved | pct_encoded | sub_delims | x3::unicode::lit(U":"));
BOOST_SPIRIT_DEFINE(userinfo);

// authority = [ userinfo "@" ] host [ ":" port ]
//
x3::rule<class unicode_authority> const authority = "unicode_authority";
auto const authority_def                          = -(userinfo >> x3::unicode::lit(U"@")) >> host >>
                           -(x3::unicode::lit(U":") >> port);
BOOST_SPIRIT_DEFINE(authority);

// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
//
x3::rule<class unicode_scheme> const scheme     = "unicode_scheme";
auto const                           scheme_def = x3::unicode::alpha >>
                        *(x3::unicode::alpha | x3::unicode::digit | x3::unicode::char_(U"+-."));
BOOST_SPIRIT_DEFINE(scheme);

// relative-part = "//" authority path-abempty
//               / path-absolute
//               / path-noscheme
//               / path-empty
//
x3::rule<class unicode_relative_part> const relative_part = "unicode_relative_part";
auto const relative_part_def = (x3::unicode::lit(U"//") >> authority >> path_abempty) |
                               path_absolute | path_noscheme | path_empty;
BOOST_SPIRIT_DEFINE(relative_part);

// relative-ref = relative-part [ "?" query ] [ "#" fragment ]
//
x3::rule<class unicode_relative_ref> const relative_ref = "unicode_relative_ref";
auto const relative_ref_def = relative_part >> -(x3::unicode::lit(U"?") >> query) >>
                              -(x3::unicode::lit(U"#") >> fragment);
BOOST_SPIRIT_DEFINE(relative_ref);

// hier-part = "//" authority path-abempty
//           / path-absolute
//           / path-rootless
//           / path-empty
//
x3::rule<class unicode_hier_part> const hier_part = "unicode_hier_part";
auto const hier_part_def = (x3::unicode::lit(U"//") >> authority >> path_abempty) | path_absolute |
                           path_rootless | path_empty;
BOOST_SPIRIT_DEFINE(hier_part);

// URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
//
x3::rule<class unicode_uri> const uri     = "unicode_uri";
auto const                        uri_def = scheme >> x3::unicode::lit(U":") >> hier_part >>
                     -(x3::unicode::lit(U"?") >> query) >> -(x3::unicode::lit(U"#") >> fragment);
BOOST_SPIRIT_DEFINE(uri);

// absolute-URI = scheme ":" hier-part [ "?" query ]
//
x3::rule<class unicode_absolute_uri> const absolute_uri = "unicode_absolute_uri";
auto const absolute_uri_def = scheme >> x3::unicode::lit(U":") >> hier_part >>
                              -(x3::unicode::lit(U"?") >> query);
BOOST_SPIRIT_DEFINE(absolute_uri);

// URI-reference = URI / relative-ref
//
x3::rule<class unicode_uri_reference> const uri_reference     = "unicode_uri_reference";
auto const                                  uri_reference_def = uri | relative_ref;
BOOST_SPIRIT_DEFINE(uri_reference);
} // namespace unicode
} // namespace uri
} // namespace foxy

#endif // FOXY_URI_HPP_
