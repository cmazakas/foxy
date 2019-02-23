//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
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
#include <array>
#include <vector>
#include <memory>
#include <iterator>

namespace foxy
{
namespace detail
{
// export_connect_fields writes all the hop-by-hop headers in `src` to the Fields container denoted
// by `dst`
//
// Using the passage below, we know the user is allowed to specify Connection multiple times in the
// fields
//
// HTTP RFC 7230: https://tools.ietf.org/html/rfc7230#section-3.2.2
// A sender MUST NOT generate multiple header fields with the same field name in a message unless
// either the entire field value for that header field is defined as a comma-separated list [i.e.,
// #(values)] or the header field is a well-known exception (as noted below).
//
// Connection ABNF:
// Connection = *( "," OWS ) connection-option *( OWS "," [ OWS connection-option ] )
//
template <class Allocator>
void
export_connect_fields(boost::beast::http::basic_fields<Allocator>& src,
                      boost::beast::http::basic_fields<Allocator>& dst);

} // namespace detail
} // namespace foxy

template <class Allocator>
void
foxy::detail::export_connect_fields(boost::beast::http::basic_fields<Allocator>& src,
                                    boost::beast::http::basic_fields<Allocator>& dst)
{
  namespace http  = boost::beast::http;
  namespace range = boost::range;

  using string_type =
    std::basic_string<char, std::char_traits<char>,
                      typename std::allocator_traits<Allocator>::template rebind_alloc<char>>;

  // first collect all the Connection options into one coherent list
  //
  auto connect_opts =
    std::vector<string_type,
                typename std::allocator_traits<Allocator>::template rebind_alloc<string_type>>(
      src.get_allocator());

  connect_opts.reserve(128);

  auto const connect_fields = src.equal_range(http::field::connection);
  auto       out            = std::back_inserter(connect_opts);

  range::for_each(connect_fields, [&src, out](auto const& connect_field) {
    range::transform(
      http::token_list(connect_field.value()), out, [&src](auto const token_view) -> string_type {
        return string_type(token_view.begin(), token_view.end(), src.get_allocator());
      });
  });

  range::sort(connect_opts);
  range::unique(connect_opts);

  // iterate the `src` fields, moving any connect headers and the corresponding tokens to the `dst`
  // fields
  //
  auto const hop_by_hops = std::array<http::field, 11>{http::field::connection,
                                                       http::field::keep_alive,
                                                       http::field::proxy_authenticate,
                                                       http::field::proxy_authentication_info,
                                                       http::field::proxy_authorization,
                                                       http::field::proxy_connection,
                                                       http::field::proxy_features,
                                                       http::field::proxy_instruction,
                                                       http::field::te,
                                                       http::field::trailer,
                                                       http::field::transfer_encoding};

  auto const is_connect_opt =
    [&connect_opts,
     &hop_by_hops](typename http::basic_fields<Allocator>::value_type const& field) -> bool {
    if (range::find(hop_by_hops, field.name()) != hop_by_hops.end()) { return true; }

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
