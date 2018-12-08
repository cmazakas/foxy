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

} // namespace uri
} // namespace foxy

#include <foxy/detail/uri_def.hpp>

#endif // FOXY_URI_HPP_
