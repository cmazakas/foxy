//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/uri_parts.hpp>
#include <foxy/uri.hpp>

namespace x3 = boost::spirit::x3;

auto
foxy::uri_parts::scheme() const -> string_view
{
  return string_view(scheme_.begin(), scheme_.end() - scheme_.begin());
}

auto
foxy::uri_parts::host() const -> string_view
{
  return string_view(host_.begin(), host_.end() - host_.begin());
}

auto
foxy::uri_parts::port() const -> string_view
{
  return string_view(port_.begin(), port_.end() - port_.begin());
}

auto
foxy::uri_parts::path() const -> string_view
{
  return string_view(path_.begin(), path_.end() - path_.begin());
}

auto
foxy::uri_parts::query() const -> string_view
{
  return string_view(query_.begin(), query_.end() - query_.begin());
}

auto
foxy::uri_parts::fragment() const -> string_view
{
  return string_view(fragment_.begin(), fragment_.end() - fragment_.begin());
}

namespace
{
auto
parse_complete(boost::string_view const uri, foxy::uri_parts& parts) -> bool
{
  auto       iter = uri.begin();
  auto const end  = uri.end();

  auto old   = iter;
  auto match = false;

  match = x3::parse(iter, end, x3::raw[foxy::uri::scheme] >> ":", parts.scheme_);
  if (!match) { goto upcall; }

  old = iter;

  match =
    x3::parse(iter, end, x3::lit("//") >> -(foxy::uri::userinfo >> "@") >> x3::raw[foxy::uri::host],
              parts.host_);

  // if we have a valid host, check for the port and then the path-abempty portion of the
  // hier-part
  //
  if (match) {
    match = x3::parse(iter, end, -(":" >> x3::raw[foxy::uri::port]), parts.port_);
    if (!match) { goto upcall; }

    match = x3::parse(iter, end, x3::raw[foxy::uri::path_abempty], parts.path_);
    if (!match) { goto upcall; }
  }

  // TODO: find out if we can ever introduce these two path parsing portions without breaking the
  // authority form parser
  //
  // if (!match) {
  //   iter = old;
  //   x3::parse(iter, end, x3::raw[foxy::uri::path_absolute()], parts.path_);
  // }

  // if (!match) {
  //   iter  = old;
  //   match = x3::parse(iter, end, x3::raw[foxy::uri::path_rootless()], parts.path_);
  // }

  if (!match) {
    iter  = old;
    match = x3::parse(iter, end, x3::raw[foxy::uri::path_empty], parts.path_);
  }

  if (!match) { goto upcall; }

  match = x3::parse(iter, end, -("?" >> x3::raw[foxy::uri::query]), parts.query_);
  if (!match) { goto upcall; }

  match = x3::parse(iter, end, -("#" >> x3::raw[foxy::uri::fragment]), parts.fragment_);

  if (!match) { goto upcall; }

  return iter == end;

upcall:
  return false;
}

auto
parse_authority(boost::string_view const uri, foxy::uri_parts& parts) -> bool
{
  auto       iter = uri.begin();
  auto const end  = uri.end();

  auto old   = iter;
  auto match = false;

  match =
    x3::parse(iter, end, -(foxy::uri::userinfo >> "@") >> x3::raw[foxy::uri::host], parts.host_);

  // if we have a valid host, check for the port and then the path-abempty portion of the
  // hier-part
  //
  if (match) {
    match = x3::parse(iter, end, -(":" >> x3::raw[foxy::uri::port]), parts.port_);
    if (!match) { goto upcall; }

    match = x3::parse(iter, end, x3::raw[foxy::uri::path_abempty], parts.path_);
    if (!match) { goto upcall; }
  }

  if (!match) { goto upcall; }

  match = x3::parse(iter, end, -("?" >> x3::raw[foxy::uri::query]), parts.query_);
  if (!match) { goto upcall; }

  match = x3::parse(iter, end, -("#" >> x3::raw[foxy::uri::fragment]), parts.fragment_);

  if (!match) { goto upcall; }

  return iter == end;

upcall:
  return false;
}

} // namespace

auto
foxy::parse_uri(boost::string_view const uri) -> foxy::uri_parts
{
  auto parts = foxy::uri_parts();
  if (parse_complete(uri, parts)) { return parts; }

  parts = foxy::uri_parts();
  if (parse_authority(uri, parts)) { return parts; }

  return foxy::uri_parts();
}

auto
foxy::uri_parts::is_http() const noexcept -> bool
{
  return scheme() == "http" || scheme() == "https";
}

auto
foxy::uri_parts::is_authority() const noexcept -> bool
{
  return scheme().empty() && !host().empty() && path().empty() && query().empty() &&
         fragment().empty();
}

auto
foxy::uri_parts::is_absolute() const noexcept -> bool
{
  return !scheme().empty() && fragment().empty();
}
