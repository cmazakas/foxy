//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/uri_parts.hpp>

auto
foxy::uri_parts::scheme() const -> string_view
{
  return scheme_;
}

auto
foxy::uri_parts::host() const -> string_view
{
  return host_;
}

auto
foxy::uri_parts::port() const -> string_view
{
  return port_;
}

auto
foxy::uri_parts::path() const -> string_view
{
  return path_;
}

auto
foxy::uri_parts::query() const -> string_view
{
  return query_;
}

auto
foxy::uri_parts::fragment() const -> string_view
{
  return fragment_;
}

auto
foxy::make_uri_parts(uri_parts::string_view const uri_view) -> uri_parts
{
  return foxy::uri_parts();
}
