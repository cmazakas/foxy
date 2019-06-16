# `foxy::uri`

Foxy comes with a large set of URI combinators compatible with [Boost.Spirit.X3](https://www.boost.org/doc/libs/release/libs/spirit/doc/x3/html/index.html).

They are a direct port of the ABNF defined in [RFC 3986](https://tools.ietf.org/html/rfc3986#appendix-A).

## Combinators

These are defined under `namespace foxy::uri` and `namespace foxy::uri::unicode`.

```c++
// sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
//
x3::rule<class sub_delims> const sub_delims     = "sub_delims";

// gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"
//
x3::rule<class gen_delims> const gen_delims     = "gen_delims";

// reserved = gen-delims / sub-delims
//
x3::rule<class reserved> const reserved     = "reserved";

// unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"
//
x3::rule<class unreserved> const unreserved     = "unreserved";

// pct-encoded = "%" HEXDIG HEXDIG
//
x3::rule<class pct_encoded> const pct_encoded     = "pct_encoded";

// pchar = unreserved / pct-encoded / sub-delims / ":" / "@"
//
x3::rule<class pchar> const pchar     = "pchar";

// query = *( pchar / "/" / "?" )
//
x3::rule<class query> const query     = "query";

// fragment = *( pchar / "/" / "?" )
//
x3::rule<class fragment> const fragment     = "fragment";

// segment = *pchar
//
x3::rule<class segment> const segment     = "segment";

// segment-nz = 1*pchar
//
x3::rule<class segment_nz> const segment_nz     = "segment_nz";

// segment-nz-nc = 1*( unreserved / pct-encoded / sub-delims / "@" )
//  ; non-zero-length segment without any colon ":"
//
x3::rule<class segment_nz_nc> const segment_nz_nc = "segment_nz_nc";

// path-empty = 0<pchar>
//
x3::rule<class path_empty> const path_empty     = "path_empty";

// path-rootless = segment-nz *( "/" segment )
//
x3::rule<class path_rootless> const path_rootless     = "path_rootless";

// path-noscheme = segment-nz-nc *( "/" segment )
//
x3::rule<class path_noscheme> const path_noscheme     = "path_noscheme";

// path-absolute = "/" [ segment-nz *( "/" segment ) ]
//
x3::rule<class path_absolute> const path_absolute = "path_absolute";

// path-abempty = *( "/" segment )
//
x3::rule<class path_abempty> const path_abempty     = "path_abempty";

// path = path-abempty    ; begins with "/" or is empty
//      / path-absolute   ; begins with "/" but not "//"
//      / path-noscheme   ; begins with a non-colon segment
//      / path-rootless   ; begins with a segment
//      / path-empty      ; zero characters
//
x3::rule<class path> const path     = "path";

// reg-name = *( unreserved / pct-encoded / sub-delims )
//
x3::rule<class reg_name> const reg_name     = "reg_name";

//    dec-octet = DIGIT                 ; 0-9
//              / %x31-39 DIGIT         ; 10-99
//              / "1" 2DIGIT            ; 100-199
//              / "2" %x30-34 DIGIT     ; 200-249
//              / "25" %x30-35          ; 250-255
//
x3::rule<class dec_octet> const dec_octet = "dec_octet";

// IPv4address = dec-octet "." dec-octet "." dec-octet "." dec-octet
//
x3::rule<class ip_v4_address> const ip_v4_address = "ip_v4_address";

// h16 = 1*4HEXDIG
//
x3::rule<class h16> const h16     = "h16";

// ls32 = ( h16 ":" h16 ) / IPv4address
//
x3::rule<class ls32> const ls32     = "ls32";

//  IPv6address   =                            6( h16 ":" ) ls32
//                /                       "::" 5( h16 ":" ) ls32
//                / [               h16 ] "::" 4( h16 ":" ) ls32
//                / [ *1( h16 ":" ) h16 ] "::" 3( h16 ":" ) ls32
//                / [ *2( h16 ":" ) h16 ] "::" 2( h16 ":" ) ls32
//                / [ *3( h16 ":" ) h16 ] "::"    h16 ":"   ls32
//                / [ *4( h16 ":" ) h16 ] "::"              ls32
//                / [ *5( h16 ":" ) h16 ] "::"              h16
//                / [ *6( h16 ":" ) h16 ] "::"
x3::rule<class ip_v6_address> const ip_v6_address = "ip_v6_address";

// IPvFuture = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )
//
x3::rule<class ip_vfuture> const ip_vfuture = "ip_vfuture";

// IP-literal = "[" ( IPv6address / IPvFuture  ) "]"
//
x3::rule<class ip_literal> const ip_literal = "ip_literal";

// port = *DIGIT
//
x3::rule<class port> const port     = "port";

// host = IP-literal / IPv4address / reg-name
//
x3::rule<class host> const host     = "host";

// userinfo = *( unreserved / pct-encoded / sub-delims / ":" )
//
x3::rule<class userinfo> const userinfo     = "userinfo";

// authority = [ userinfo "@" ] host [ ":" port ]
//
x3::rule<class authority> const authority     = "authority";

// scheme = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
//
x3::rule<class scheme> const scheme     = "scheme";

// relative-part = "//" authority path-abempty
//               / path-absolute
//               / path-noscheme
//               / path-empty
//
x3::rule<class relative_part> const relative_part = "relative_part";

// relative-ref = relative-part [ "?" query ] [ "#" fragment ]
//
x3::rule<class relative_ref> const relative_ref = "relative_ref";

// hier-part = "//" authority path-abempty
//           / path-absolute
//           / path-rootless
//           / path-empty
//
x3::rule<class hier_part> const hier_part = "hier_part";

// URI = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
//
x3::rule<class uri> const uri = "uri";

// absolute-URI = scheme ":" hier-part [ "?" query ]
//
x3::rule<class absolute_uri> const absolute_uri     = "absolute_uri";

// URI-reference = URI / relative-ref
//
x3::rule<class uri_reference> const uri_reference     = "uri_reference";
```

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
