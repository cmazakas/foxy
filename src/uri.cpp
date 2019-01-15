//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

// TODO: someday get this working
// The problem:
//
// x3::parse(begin, end, foxy::uri::unreserved(), unused) compiles
// x3::parse(begin, end, (foxy::uri::unreserved()) >> ",", unused); fails
// with a compiler error of unresolved external symbols
//
// find out why X3 chokes so hard on the explicitly compiled template
// instantiations
//

#include <foxy/uri.hpp>
#include <foxy/detail/uri_def.hpp>

namespace foxy
{
namespace uri
{
namespace parser
{
using iterator_type = char const*;
using context_type  = x3::unused_type;

BOOST_SPIRIT_INSTANTIATE(sub_delims_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(gen_delims_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(reserved_type, iterator_type, context_type);
BOOST_SPIRIT_INSTANTIATE(unreserved_type, iterator_type, context_type);
} // namespace parser
} // namespace uri
} // namespace foxy
