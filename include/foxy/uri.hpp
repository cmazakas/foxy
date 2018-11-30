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

namespace foxy
{
namespace uri
{
namespace parser
{
using sub_delims_type = boost::spirit::x3::rule<class sub_delims>;
BOOST_SPIRIT_DECLARE(sub_delims_type);
} // namespace parser

auto
sub_delims() -> parser::sub_delims_type;

} // namespace uri
} // namespace foxy

#endif // FOXY_URI_HPP_
