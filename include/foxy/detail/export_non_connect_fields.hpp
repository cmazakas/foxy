//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_
#define FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/type_traits.hpp>

#include <boost/beast/http/rfc7230.hpp>

#include <boost/utility/string_view.hpp>
#include <boost/range/end.hpp>
#include <boost/range/algorithm.hpp>

#include <type_traits>
#include <vector>
#include <memory>
#include <iterator>

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

// Using this, we know the user is allowed to specify Connection multiple
// times in the fields
//
// HTTP RFC 7230: https://tools.ietf.org/html/rfc7230#section-3.2.2
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
  // first collect all the Connection options into one coherent list
  //
  namespace http = boost::beast::http;
  namespace rng  = boost::range;

  using allocator_type = typename std::allocator_traits<
    typename Fields::allocator_type
  >::template rebind_alloc<boost::string_view>;

  auto connect_opts =
    std::vector<boost::string_view, allocator_type>(src.get_allocator());
  connect_opts.reserve(128);

  auto const connect_fields = src.equal_range(http::field::connection);

  auto out = std::back_inserter(connect_opts);

  rng::for_each(
    connect_fields,
    [out]
    (auto const& connect_field)
    {
      rng::copy(http::token_list(connect_field.value()), out);
    });

  rng::sort(connect_opts);
  rng::unique(connect_opts);

  // iterate the `src` fields, moving any non-connect headers and the
  // corresponding tokens to the `dst` fields
  //
  auto const is_connect_opt =
    [&connect_opts]
    (typename Fields::value_type const& field) -> bool
    {
      if (field.name() == http::field::connection) {
        return true;
      }
      return rng::find(connect_opts, field.name_string()) != boost::end(connect_opts);
    };

  for (auto it = src.begin(); it != src.end(); ++it) {
    auto const& field = *it;
    if (is_connect_opt(field)) {
      continue;
    }
    dst.insert(field.name_string(), field.value());
    it = src.erase(it);
    if (it == src.end()) {
      break;
    }
  }
}

#endif // FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_
