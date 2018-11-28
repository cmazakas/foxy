//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_DETAIL_PARSE_HOST_AND_PATH_HPP_
#define FOXY_DETAIL_PARSE_HOST_AND_PATH_HPP_

#include <boost/spirit/home/x3.hpp>

#include <boost/fusion/adapted/std_tuple.hpp>
#include <boost/fusion/include/std_tuple.hpp>

#include <boost/utility/string_view.hpp>

#include <tuple>

namespace foxy
{
namespace uri
{
static auto const sub_delims =
  boost::spirit::x3::lit("!") | boost::spirit::x3::lit("$") |
  boost::spirit::x3::lit("&") | boost::spirit::x3::lit("'") |
  boost::spirit::x3::lit("(") | boost::spirit::x3::lit(")") |
  boost::spirit::x3::lit("*") | boost::spirit::x3::lit("+") |
  boost::spirit::x3::lit(",") | boost::spirit::x3::lit(";") |
  boost::spirit::x3::lit("=");
}

namespace detail
{
template <class String>
auto
parse_host_and_path(boost::string_view const              uri,
                    std::tuple<String&, String&, String&> attribute) -> void;
} // namespace detail
} // namespace foxy

template <class String>
auto
foxy::detail::parse_host_and_path(
  boost::string_view const              uri,
  std::tuple<String&, String&, String&> attribute) -> void
{
  namespace x3 = boost::spirit::x3;

  // auto chars = x3::char_set<boost::spirit::char_encoding::ascii>("123");

  using char_set = x3::char_set<boost::spirit::char_encoding::ascii>;

  auto const sub_delims = x3::lit("!") | x3::lit("$") | x3::lit("&") |
                          x3::lit("'") | x3::lit("(") | x3::lit(")") |
                          x3::lit("*") | x3::lit("+") | x3::lit(",") |
                          x3::lit(";") | x3::lit("=");

  auto const gen_delims = x3::lit(":") | x3::lit("|") | x3::lit("?") |
                          x3::lit("#") | x3::lit("[") | x3::lit("]") |
                          x3::lit("@");

  auto const reserved   = gen_delims | sub_delims;
  auto const unreserved = x3::alpha | x3::digit | "-" | "." | "_" | "~";

  auto const pct_encoded = x3::lit("%") >> x3::hex >> x3::hex;

  auto const pchar = unreserved | pct_encoded | sub_delims | ":" | "@";

  auto const query    = *(pchar | "/" | "?");
  auto const fragment = *(pchar | "/" | "?");

  auto const segment       = *pchar;
  auto const segment_nz    = pchar;
  auto const segment_nz_nc = unreserved | pct_encoded | sub_delims | "@";

  auto const path_abempty  = *("/" >> segment);
  auto const path_absolute = "/" >> -(segment_nz >> *("/" >> segment));
  auto const path_noscheme = segment_nz_nc >> *("/" >> segment);
  auto const path_rootless = segment_nz >> *("/" >> segment);
  auto const path_empty    = x3::eps;

  auto const path =
    path_abempty | path_absolute | path_noscheme | path_rootless | path_empty;

  auto const reg_name = *(unreserved | pct_encoded | sub_delims);

  auto const dec_octet = x3::digit                               // 0-9
                         | (char_set("1-9") >> x3::digit)        // 10-99
                         | ("1" >> x3::digit >> x3::digit)       // 100-199
                         | ("2" >> char_set("0-4") >> x3::digit) // 200-249
                         | ("25" >> char_set("0-5"));            // 250-255

  auto       begin = uri.begin();
  auto const end   = uri.end();

  x3::parse(begin, end,
            x3::lit("http") >> -x3::lit("s") >> "://" >> +(x3::char_ - ":") >>
              -(":" >> +(x3::char_ - "/")) >> +x3::char_,
            attribute);
}

#endif // FOXY_DETAIL_PARSE_HOST_AND_PATH_HPP_
