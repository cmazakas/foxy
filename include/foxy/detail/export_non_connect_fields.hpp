//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#ifndef FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_
#define FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_

#include <boost/beast/http/type_traits.hpp>
#include <type_traits>

namespace foxy
{
namespace detail
{

template <class Fields>
void
export_non_connect_fields(Fields& src, Fields& dst);

} // detail
} // foxy

template <class Fields>
void
foxy::detail::export_non_connect_fields(Fields& src, Fields& dst)
{
}

#endif // FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_
