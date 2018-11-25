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

  auto       begin = uri.begin();
  auto const end   = uri.end();

  x3::parse(begin, end,
            x3::lit("http") >> -x3::lit("s") >> "://" >> +(x3::char_ - ":") >>
              -(":" >> +(x3::char_ - "/")) >> +x3::char_,
            attribute);
}

#endif // FOXY_DETAIL_PARSE_HOST_AND_PATH_HPP_
