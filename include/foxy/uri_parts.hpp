//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <boost/utility/string_view.hpp>
#include <boost/range/iterator_range_core.hpp>
#include <boost/fusion/adapted/struct/adapt_struct.hpp>
#include <boost/fusion/include/adapt_struct.hpp>

namespace foxy
{
// uri_parts stores a set of `string_views` that identify each relevant portion of a URI as defined
// by the RFC
// uri_parts is not meant to be consumed directly and instead should be created by the factory
// function `foxy::make_uri_parts`.
//
struct uri_parts
{
public:
  using string_view = boost::string_view;
  using iterator    = typename string_view::iterator;
  using range       = boost::iterator_range<iterator>;

  // these are kept public so that the uri_parts class can be used as a Fusion ForwardSequence
  //
  range scheme_;
  range host_;
  range port_;
  range path_;
  range query_;
  range fragment_;

  uri_parts()                 = delete;
  uri_parts(uri_parts const&) = default;
  uri_parts(uri_parts&&)      = default;

  template <class Range>
  uri_parts(Range const& range)
    : scheme_(range.begin(), range.begin())
    , host_(range.begin(), range.begin())
    , port_(range.begin(), range.begin())
    , path_(range.begin(), range.begin())
    , query_(range.begin(), range.begin())
    , fragment_(range.begin(), range.begin())
  {
  }

  auto
  operator=(uri_parts const&) -> uri_parts& = default;

  auto
  operator=(uri_parts&&) noexcept -> uri_parts& = default;

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

  // is_http returns whether or not the given URI has a scheme of either "http" or "https"
  //
  auto
  is_http() const noexcept -> bool;

  // is_authority returns whether or not the URI is in its authority form, i.e. the URI follows the
  // ABNF of: authority path-abempty [ "?" query ] [ "#" fragment ]
  // this method is useful when designing servers that support the CONNECT method
  //
  auto
  is_authority() const noexcept -> bool;

  // is_absolute returns whether or not the URI is of the form defined by the ABNF:
  // absolute-URI  = scheme ":" hier-part [ "?" query ]
  //
  auto
  is_absolute() const noexcept -> bool;
};

auto
make_uri_parts(uri_parts::string_view const uri_view) -> uri_parts;

} // namespace foxy

BOOST_FUSION_ADAPT_STRUCT(foxy::uri_parts, scheme_, host_, port_, path_, query_, fragment_)
