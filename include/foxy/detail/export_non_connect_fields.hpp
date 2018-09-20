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

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/type_traits.hpp>
#include <boost/utility/string_view.hpp>
#include <boost/range/algorithm/for_each.hpp>
#include <type_traits>
#include <vector>

namespace foxy
{
namespace detail
{

template <
  class Fields,
  class = std::enable_if_t<boost::beast::http::is_fields<Fields>::value>
>
void
export_non_connect_fields(Fields& src, Fields& dst);

} // detail
} // foxy

// HTTP RFC 7230: https://tools.ietf.org/html/rfc7230#section-3.2.2
//
// A sender MUST NOT generate multiple header fields with the same field
// name in a message unless either the entire field value for that
// header field is defined as a comma-separated list [i.e., #(values)]
// or the header field is a well-known exception (as noted below).
//
// Connection ABNF:
// Connection = *( "," OWS ) connection-option *( OWS "," [ OWS connection-option ] )
//
template <class Fields, class X>
void
foxy::detail::
export_non_connect_fields(Fields& src, Fields& dst)
{
  namespace http = boost::beast::http;

  // first collect all the Connection options into one coherent list
  //
  auto const connect_fields = src.equal_range(http::field::connection);

  auto connect_token_names =
    std::vector<
      boost::string_view,
      typename Fields::allocator_type
    >(src.get_allocator());

  connect_token_names.reserve(128);

  boost::range::for_each(
    connect_fields,
    [](auto const& connect_field)
    {

    });

  // iterate the `src` fields, moving any non-connect headers and the
  // corresponding tokens to the `dst` fields
  //
}

#endif // FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_
