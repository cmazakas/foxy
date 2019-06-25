//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_PCT_DECODE_HPP_
#define FOXY_PCT_DECODE_HPP_

#define BOOST_SPIRIT_X3_UNICODE

#include <foxy/error.hpp>

#include <boost/utility/string_view.hpp>

#include <boost/fusion/adapted/std_pair.hpp>
#include <boost/fusion/include/std_pair.hpp>

#include <boost/spirit/home/x3.hpp>

#include <utility>

namespace foxy
{
namespace uri
{
namespace x3 = boost::spirit::x3;

x3::rule<struct xpair, std::pair<char, char>> const xpair     = "xpair";
auto const                                          xpair_def = "%" >> x3::xdigit >> x3::xdigit;
BOOST_SPIRIT_DEFINE(xpair);

namespace detail
{
template <class OutputIterator>
auto
pct_decode_impl(boost::string_view const str, OutputIterator& sink) -> bool
{
  auto pos = str.begin();

  auto const to_utf8_byte = [&sink](auto& ctx) mutable {
    auto const xdigit_pair = _attr(ctx);

    auto const first_xdigit  = std::get<0>(xdigit_pair);
    auto const second_xdigit = std::get<1>(xdigit_pair);

    auto const first_char = (first_xdigit <= '9' ? first_xdigit - '0' : first_xdigit - 'a' + 10);
    auto const second_char =
      (second_xdigit <= '9' ? second_xdigit - '0' : second_xdigit - 'a' + 10);

    *sink++ = (first_char << 4) + second_char;
  };

  auto const append_byte = [&sink](auto& ctx) mutable { *sink++ = _attr(ctx); };

  auto const match =
    x3::parse(pos, str.end(), *(xpair[to_utf8_byte] | (x3::char_ - "%")[append_byte]));

  return match && pos == str.end();
}
} // namespace detail

template <class OutputIterator>
auto
pct_decode(boost::string_view const str, OutputIterator& sink) -> void
{
  auto const full_match = detail::pct_decode_impl(str, sink);
  if (!full_match) { throw ::foxy::make_error_code(::foxy::error::unexpected_pct); }
}

template <class OutputIterator>
auto
pct_decode(boost::string_view const   str,
           OutputIterator             sink,
           boost::system::error_code& ec) noexcept -> OutputIterator
{
  auto const full_match = detail::pct_decode_impl(str, sink);

  ec = !full_match ? ::foxy::make_error_code(::foxy::error::unexpected_pct)
                   : boost::system::error_code{};

  return sink;
}

} // namespace uri
} // namespace foxy

#endif // FOXY_PCT_DECODE_HPP_
