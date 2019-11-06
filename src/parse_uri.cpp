//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/parse_uri.hpp>
#include <foxy/uri.hpp>

namespace x3 = boost::spirit::x3;

auto
foxy::parse_uri(boost::basic_string_view<char, std::char_traits<char>> const uri)
  -> foxy::basic_uri_parts<char>
{
  auto parts = foxy::basic_uri_parts<char>();
  if (foxy::parse_complete(uri, parts)) { return parts; }

  parts = foxy::basic_uri_parts<char>();
  if (foxy::parse_authority(uri, parts)) { return parts; }

  return foxy::basic_uri_parts<char>();
}

auto
foxy::parse_complete(boost::basic_string_view<char, std::char_traits<char>> const uri,
                     foxy::basic_uri_parts<char>&                                 parts) -> bool
{
  auto       iter = uri.begin();
  auto const end  = uri.end();

  auto old   = iter;
  auto match = false;

  match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::scheme] >> ":", parts.scheme_);
  if (!match) { return false; }

  old = iter;

  match = x3::parse(
    iter, end, x3::lit("//") >> -(foxy::uri::userinfo >> "@") >> foxy::uri::raw[foxy::uri::host],
    parts.host_);

  // if we have a valid host, check for the port and then the path-abempty portion of the
  // hier-part
  //
  if (match) {
    match = x3::parse(iter, end, -(":" >> foxy::uri::raw[foxy::uri::port]), parts.port_);
    if (!match) { return false; }

    match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::path_abempty], parts.path_);
    if (!match) { return false; }
  }

  // TODO: find out if we can ever introduce these two path parsing portions without breaking the
  // authority form parser
  //
  // if (!match) {
  //   iter = old;
  //   x3::parse(iter, end, foxy::uri::raw[foxy::uri::path_absolute()], parts.path_);
  // }

  // if (!match) {
  //   iter  = old;
  //   match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::path_rootless()], parts.path_);
  // }

  if (!match) {
    iter  = old;
    match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::path_empty], parts.path_);
  }

  if (!match) { return false; }

  match = x3::parse(iter, end, -("?" >> foxy::uri::raw[foxy::uri::query]), parts.query_);
  if (!match) { return false; }

  match = x3::parse(iter, end, -("#" >> foxy::uri::raw[foxy::uri::fragment]), parts.fragment_);
  if (!match) { return false; }

  return iter == end;
}

auto
foxy::parse_authority(boost::basic_string_view<char, std::char_traits<char>> const uri,
                      foxy::basic_uri_parts<char>&                                 parts) -> bool
{
  auto       iter = uri.begin();
  auto const end  = uri.end();

  auto old   = iter;
  auto match = false;

  match = x3::parse(iter, end, -(foxy::uri::userinfo >> "@") >> foxy::uri::raw[foxy::uri::host],
                    parts.host_);

  // if we have a valid host, check for the port and then the path-abempty portion of the
  // hier-part
  //
  if (match) {
    match = x3::parse(iter, end, -(":" >> foxy::uri::raw[foxy::uri::port]), parts.port_);
    if (!match) { return false; }

    match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::path_abempty], parts.path_);
    if (!match) { return false; }
  }

  if (!match) { return false; }

  match = x3::parse(iter, end, -("?" >> foxy::uri::raw[foxy::uri::query]), parts.query_);
  if (!match) { return false; }

  match = x3::parse(iter, end, -("#" >> foxy::uri::raw[foxy::uri::fragment]), parts.fragment_);
  if (!match) { return false; }

  return iter == end;
}

auto
foxy::parse_uri(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri)
  -> foxy::basic_uri_parts<char32_t>
{
  auto parts = foxy::basic_uri_parts<char32_t>();
  if (foxy::parse_complete(uri, parts)) { return parts; }

  parts = foxy::basic_uri_parts<char32_t>();
  if (foxy::parse_authority(uri, parts)) { return parts; }

  return foxy::basic_uri_parts<char32_t>();
}

auto
foxy::parse_complete(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri,
                     foxy::basic_uri_parts<char32_t>& parts) -> bool
{
  auto       iter = uri.begin();
  auto const end  = uri.end();

  auto old   = iter;
  auto match = false;

  match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::unicode::scheme] >> x3::unicode::lit(U":"),
                    parts.scheme_);
  if (!match) { return false; }

  old = iter;

  match = x3::parse(iter, end,
                    x3::unicode::lit(U"//") >>
                      -(foxy::uri::unicode::userinfo >> x3::unicode::lit(U"@")) >>
                      foxy::uri::raw[foxy::uri::unicode::host],
                    parts.host_);

  // if we have a valid host, check for the port and then the path-abempty portion of the
  // hier-part
  //
  if (match) {
    match =
      x3::parse(iter, end, -(x3::unicode::lit(U":") >> foxy::uri::raw[foxy::uri::unicode::port]),
                parts.port_);
    if (!match) { return false; }

    match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::unicode::path_abempty], parts.path_);
    if (!match) { return false; }
  }

  // TODO: find out if we can ever introduce these two path parsing portions without breaking the
  // authority form parser
  //
  // if (!match) {
  //   iter = old;
  //   x3::parse(iter, end, foxy::uri::raw[foxy::uri::path_absolute()], parts.path_);
  // }

  // if (!match) {
  //   iter  = old;
  //   match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::path_rootless()], parts.path_);
  // }

  if (!match) {
    iter  = old;
    match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::unicode::path_empty], parts.path_);
  }

  if (!match) { return false; }

  match =
    x3::parse(iter, end, -(x3::unicode::lit(U"?") >> foxy::uri::raw[foxy::uri::unicode::query]),
              parts.query_);
  if (!match) { return false; }

  match =
    x3::parse(iter, end, -(x3::unicode::lit(U"#") >> foxy::uri::raw[foxy::uri::unicode::fragment]),
              parts.fragment_);

  if (!match) { return false; }

  return iter == end;
}

auto
foxy::parse_authority(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri,
                      foxy::basic_uri_parts<char32_t>& parts) -> bool
{
  auto       iter = uri.begin();
  auto const end  = uri.end();

  auto old   = iter;
  auto match = false;

  match = x3::parse(iter, end,
                    -(foxy::uri::unicode::userinfo >> x3::unicode::lit(U"@")) >>
                      foxy::uri::raw[foxy::uri::unicode::host],
                    parts.host_);

  // if we have a valid host, check for the port and then the path-abempty portion of the
  // hier-part
  //
  if (match) {
    match =
      x3::parse(iter, end, -(x3::unicode::lit(U":") >> foxy::uri::raw[foxy::uri::unicode::port]),
                parts.port_);
    if (!match) { return false; }

    match = x3::parse(iter, end, foxy::uri::raw[foxy::uri::unicode::path_abempty], parts.path_);
    if (!match) { return false; }
  }

  if (!match) { return false; }

  match =
    x3::parse(iter, end, -(x3::unicode::lit(U"?") >> foxy::uri::raw[foxy::uri::unicode::query]),
              parts.query_);
  if (!match) { return false; }

  match =
    x3::parse(iter, end, -(x3::unicode::lit(U"#") >> foxy::uri::raw[foxy::uri::unicode::fragment]),
              parts.fragment_);

  if (!match) { return false; }

  return iter == end;
}
