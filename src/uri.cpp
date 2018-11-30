//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/detail/uri_def.hpp>

namespace foxy
{
namespace uri
{
namespace parser
{
using iterator_type = char const*;
using context_type  = boost::spirit::x3::unused_type;
BOOST_SPIRIT_INSTANTIATE(sub_delims_type, iterator_type, context_type);
} // namespace parser
} // namespace uri
} // namespace foxy
