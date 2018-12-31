//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <boost/utility/string_view.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace foxy
{
struct uri_parts
{
public:
  using string_view = boost::string_view;
  using iterator    = typename string_view::iterator;
  using range       = boost::iterator_range<iterator>;

  range scheme_;
  range host_;
  range port_;
  range path_;
  range query_;
  range fragment_;

  uri_parts()                 = default;
  uri_parts(uri_parts const&) = default;
  uri_parts(uri_parts&&)      = default;

  auto
  scheme() const -> string_view;

  auto
  host() const -> string_view;

  auto
  port() const -> string_view;

  auto
  path() const -> string_view;

  auto
  query() const -> string_view;

  auto
  fragment() const -> string_view;

  auto
  is_http() const noexcept -> bool;

  auto
  is_authority() const noexcept -> bool;
};

auto
make_uri_parts(uri_parts::string_view const uri_view) -> uri_parts;

} // namespace foxy

BOOST_FUSION_ADAPT_STRUCT(foxy::uri_parts, scheme_, host_, port_, path_, query_, fragment_)
