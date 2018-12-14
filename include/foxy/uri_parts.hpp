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

namespace foxy
{
struct uri_parts
{
public:
  using string_view = boost::string_view;

private:
  string_view scheme_;
  string_view host_;
  string_view port_;
  string_view path_;
  string_view query_;
  string_view fragment_;

public:
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
};

auto
make_uri_parts(uri_parts::string_view const uri_view) -> uri_parts;

} // namespace foxy
