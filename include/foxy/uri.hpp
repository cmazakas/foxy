//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_URI_HPP_
#define FOXY_URI_HPP_

#include <boost/spirit/home/x3.hpp>
#include <boost/utility/string_view.hpp>

namespace foxy
{
namespace uri
{
namespace parser
{
namespace x3 = boost::spirit::x3;

using sub_delims_type = x3::rule<class sub_delims>;
BOOST_SPIRIT_DECLARE(sub_delims_type);

using gen_delims_type = x3::rule<class gen_delims>;
BOOST_SPIRIT_DECLARE(gen_delims_type);

using reserved_type = x3::rule<class reserved>;
BOOST_SPIRIT_DECLARE(reserved_type);

using unreserved_type = x3::rule<class unreserved>;
BOOST_SPIRIT_DECLARE(unreserved_type);

using pct_encoded_type = x3::rule<class pct_encoded>;
BOOST_SPIRIT_DECLARE(pct_encoded_type);

using pchar_type = x3::rule<class pchar>;
BOOST_SPIRIT_DECLARE(pchar_type);

using query_type = x3::rule<class query>;
BOOST_SPIRIT_DECLARE(query_type);

using fragment_type = x3::rule<class fragment>;
BOOST_SPIRIT_DECLARE(fragment_type);

using segment_type = x3::rule<class segment>;
BOOST_SPIRIT_DECLARE(segment_type);

using segment_nz_type = x3::rule<class segment_nz>;
BOOST_SPIRIT_DECLARE(segment_nz_type);

using segment_nz_nc_type = x3::rule<class segment_nz_nc>;
BOOST_SPIRIT_DECLARE(segment_nz_nc_type);

using path_empty_type = x3::rule<class path_empty>;
BOOST_SPIRIT_DECLARE(path_empty_type);

using path_rootless_type = x3::rule<class path_rootless>;
BOOST_SPIRIT_DECLARE(path_rootless_type);

using path_noscheme_type = x3::rule<class path_noscheme>;
BOOST_SPIRIT_DECLARE(path_noscheme_type);

using path_absolute_type = x3::rule<class path_absolute>;
BOOST_SPIRIT_DECLARE(path_absolute_type);

using path_abempty_type = x3::rule<class path_abempty>;
BOOST_SPIRIT_DECLARE(path_abempty_type);

using path_type = x3::rule<class path>;
BOOST_SPIRIT_DECLARE(path_type);

using reg_name_type = x3::rule<class reg_name>;
BOOST_SPIRIT_DECLARE(reg_name_type);

using dec_octet_type = x3::rule<class dec_octet>;
BOOST_SPIRIT_DECLARE(dec_octet_type);

using ip_v4_address_type = x3::rule<class ip_v4_address>;
BOOST_SPIRIT_DECLARE(ip_v4_address_type);

using h16_type = x3::rule<class h16>;
BOOST_SPIRIT_DECLARE(h16_type);

using ls32_type = x3::rule<class ls32>;
BOOST_SPIRIT_DECLARE(ls32_type);

using ip_v6_address_type = x3::rule<class ip_v6_address>;
BOOST_SPIRIT_DECLARE(ip_v6_address_type);

using ip_vfuture_type = x3::rule<class ip_vfuture>;
BOOST_SPIRIT_DECLARE(ip_vfuture_type);

using ip_literal_type = x3::rule<class ip_literal>;
BOOST_SPIRIT_DECLARE(ip_literal_type);

using port_type = x3::rule<class port>;
BOOST_SPIRIT_DECLARE(port_type);

using host_type = x3::rule<class host>;
BOOST_SPIRIT_DECLARE(host_type);

using userinfo_type = x3::rule<class userinfo>;
BOOST_SPIRIT_DECLARE(userinfo_type);

using authority_type = x3::rule<class authority>;
BOOST_SPIRIT_DECLARE(authority_type);

using scheme_type = x3::rule<class scheme>;
BOOST_SPIRIT_DECLARE(scheme_type);

using relative_part_type = x3::rule<class relative_part>;
BOOST_SPIRIT_DECLARE(relative_part_type);

using relative_ref_type = x3::rule<class relative_ref>;
BOOST_SPIRIT_DECLARE(relative_ref_type);

using absolute_uri_type = x3::rule<class absolute_uri>;
BOOST_SPIRIT_DECLARE(absolute_uri_type);

using uri_reference_type = x3::rule<class uri_reference>;
BOOST_SPIRIT_DECLARE(uri_reference_type);

using hier_part_type = x3::rule<class hier_part>;
BOOST_SPIRIT_DECLARE(hier_part_type);

using uri_type = x3::rule<class uri>;
BOOST_SPIRIT_DECLARE(uri_type);

} // namespace parser

auto
sub_delims() -> parser::sub_delims_type;

auto
gen_delims() -> parser::gen_delims_type;

auto
reserved() -> parser::reserved_type;

auto
unreserved() -> parser::unreserved_type;

auto
pct_encoded() -> parser::pct_encoded_type;

auto
pchar() -> parser::pchar_type;

auto
query() -> parser::query_type;

auto
fragment() -> parser::fragment_type;

auto
segment() -> parser::segment_type;

auto
segment_nz() -> parser::segment_nz_type;

auto
segment_nz_nc() -> parser::segment_nz_nc_type;

auto
path_empty() -> parser::path_empty_type;

auto
path_rootless() -> parser::path_rootless_type;

auto
path_noscheme() -> parser::path_noscheme_type;

auto
path_absolute() -> parser::path_absolute_type;

auto
path_abempty() -> parser::path_abempty_type;

auto
path() -> parser::path_type;

auto
reg_name() -> parser::reg_name_type;

auto
dec_octet() -> parser::dec_octet_type;

auto
ip_v4_address() -> parser::ip_v4_address_type;

auto
h16() -> parser::h16_type;

auto
ls32() -> parser::ls32_type;

auto
ip_v6_address() -> parser::ip_v6_address_type;

auto
ip_vfuture() -> parser::ip_vfuture_type;

auto
ip_literal() -> parser::ip_literal_type;

auto
port() -> parser::port_type;

auto
host() -> parser::host_type;

auto
userinfo() -> parser::userinfo_type;

auto
authority() -> parser::authority_type;

auto
scheme() -> parser::scheme_type;

auto
relative_part() -> parser::relative_part_type;

auto
relative_ref() -> parser::relative_ref_type;

auto
absolute_uri() -> parser::absolute_uri_type;

auto
uri_reference() -> parser::uri_reference_type;

auto
hier_part() -> parser::hier_part_type;

auto
uri() -> parser::uri_type;

} // namespace uri
} // namespace foxy

#include <foxy/detail/uri_def.hpp>

#endif // FOXY_URI_HPP_
