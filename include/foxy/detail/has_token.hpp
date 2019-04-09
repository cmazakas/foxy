//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_HAS_TOKEN_HPP_
#define FOXY_DETAIL_HAS_TOKEN_HPP_

#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>

#include <boost/utility/string_view.hpp>

#include <boost/spirit/home/x3.hpp>

#include <algorithm>
#include <numeric>

namespace foxy
{
namespace detail
{
template <class Allocator>
auto
has_foxy_via(boost::beast::http::basic_fields<Allocator> const& fields) -> bool
{
  namespace http = boost::beast::http;
  namespace x3   = boost::spirit::x3;

  auto const field_range = fields.equal_range(http::field::via);

  auto       begin = field_range.first;
  auto const end   = field_range.second;

  auto found = false;
  for (; begin != end; ++begin) {
    auto       field_val_iter = begin->value().begin();
    auto const field_val_end  = begin->value().end();

    found = x3::parse(field_val_iter, field_val_end,
                      x3::no_case[*(x3::char_ - "1.1 foxy") >> x3::lit("1.1 foxy") >> *x3::char_]);

    if (found) { break; }
  }
  return found;
}

} // namespace detail
} // namespace foxy

#endif // FOXY_DETAIL_HAS_TOKEN_HPP_
