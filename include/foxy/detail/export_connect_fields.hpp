//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_EXPORT_CONNECT_FIELDS_HPP_
#define FOXY_DETAIL_EXPORT_CONNECT_FIELDS_HPP_

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/type_traits.hpp>

#include <boost/beast/http/rfc7230.hpp>

#include <boost/utility/string_view.hpp>
#include <boost/range/algorithm.hpp>

#include <type_traits>
#include <vector>
#include <memory>
#include <iterator>

namespace foxy
{
namespace detail
{
// export_connect_fields writes all the hop-by-hop headers in `src` to the
// Fields container denoted by `dst`
//
// Using the passage below, we know the user is allowed to specify Connection
// multiple times in the fields
//
// HTTP RFC 7230: https://tools.ietf.org/html/rfc7230#section-3.2.2
// A sender MUST NOT generate multiple header fields with the same field
// name in a message unless either the entire field value for that
// header field is defined as a comma-separated list [i.e., #(values)]
// or the header field is a well-known exception (as noted below).
//
// Connection ABNF:
// Connection = *( "," OWS ) connection-option *( OWS "," [ OWS
// connection-option ] )
//
template <
  class Fields,
  class = std::enable_if_t<boost::beast::http::is_fields<Fields>::value>>
void
export_connect_fields(Fields& src, Fields& dst);

} // namespace detail
} // namespace foxy

template <class Fields, class X>
void
foxy::detail::export_connect_fields(Fields& src, Fields& dst)
{
  namespace http  = boost::beast::http;
  namespace range = boost::range;

  using fields_allocator_type = typename Fields::allocator_type;

  using string_allocator_type = typename std::allocator_traits<
    fields_allocator_type>::template rebind_alloc<char>;

  using string_type =
    std::basic_string<char, std::char_traits<char>, string_allocator_type>;

  using vector_allocator_type = typename std::allocator_traits<
    fields_allocator_type>::template rebind_alloc<string_type>;

  // first collect all the Connection options into one coherent list
  //
  auto connect_opts =
    std::vector<string_type, vector_allocator_type>(src.get_allocator());

  connect_opts.reserve(128);

  auto const connect_fields = src.equal_range(http::field::connection);
  auto       out            = std::back_inserter(connect_opts);

  range::for_each(connect_fields, [out](auto const& connect_field) {
    range::transform(http::token_list(connect_field.value()), out,
                     [](auto const token_view) -> string_type {
                       return string_type{token_view.begin(), token_view.end()};
                     });
  });

  range::sort(connect_opts);
  range::unique(connect_opts);

  // iterate the `src` fields, moving any connect headers and the
  // corresponding tokens to the `dst` fields
  //
  auto const is_connect_opt =
    [&connect_opts](typename Fields::value_type const& field) -> bool {
    if (field.name() == http::field::connection) { return true; }
    for (auto const opt : connect_opts) {
      if (field.name_string() == opt) { return true; }
    }
    return false;
  };

  for (auto it = src.begin(); it != src.end();) {
    auto const& field = *it;

    if (!is_connect_opt(field)) {
      ++it;
      continue;
    }

    dst.insert(field.name_string(), field.value());
    it = src.erase(it);
  }
}

#endif // FOXY_DETAIL_EXPORT_CONNECT_FIELDS_HPP_
