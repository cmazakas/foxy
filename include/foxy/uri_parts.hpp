//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <boost/utility/string_view.hpp>

namespace foxy
{
template <class CharT>
struct basic_uri_parts
{
public:
  using string_view = boost::basic_string_view<CharT, std::char_traits<CharT>>;
  using iterator    = typename string_view::iterator;

  friend auto
  parse_uri(boost::basic_string_view<char, std::char_traits<char>> const uri)
    -> basic_uri_parts<char>;

  friend auto
  parse_complete(boost::basic_string_view<char, std::char_traits<char>> const uri,
                 basic_uri_parts<char>&                                       parts) -> bool;

  friend auto
  parse_authority(boost::basic_string_view<char, std::char_traits<char>> const uri,
                  basic_uri_parts<char>&                                       parts) -> bool;

  friend auto
  parse_uri(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri)
    -> basic_uri_parts<char32_t>;

  friend auto
  parse_complete(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri,
                 basic_uri_parts<char32_t>& parts) -> bool;

  friend auto
  parse_authority(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri,
                  basic_uri_parts<char32_t>& parts) -> bool;

private:
  string_view scheme_;
  string_view host_;
  string_view port_;
  string_view path_;
  string_view query_;
  string_view fragment_;

  static auto
  is_http_impl(boost::basic_string_view<char, std::char_traits<char>> const scheme) -> bool
  {
    return scheme == "http" || scheme == "https";
  }

  static auto
  is_http_impl(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const scheme) -> bool
  {
    return scheme == U"http" || scheme == U"https";
  }

public:
  basic_uri_parts()
    : scheme_(nullptr, 0)
    , host_(nullptr, 0)
    , port_(nullptr, 0)
    , path_(nullptr, 0)
    , query_(nullptr, 0)
    , fragment_(nullptr, 0)
  {
  }

  basic_uri_parts(basic_uri_parts const&) = default;
  basic_uri_parts(basic_uri_parts&&)      = default;

  auto
  operator=(basic_uri_parts const&) -> basic_uri_parts& = default;

  auto
  operator=(basic_uri_parts&&) noexcept -> basic_uri_parts& = default;

  auto
  scheme() const noexcept -> string_view
  {
    return scheme_;
  }

  auto
  host() const noexcept -> string_view
  {
    return host_;
  }

  auto
  port() const noexcept -> string_view
  {
    return port_;
  }

  auto
  path() const noexcept -> string_view
  {
    return path_;
  }

  auto
  query() const noexcept -> string_view
  {
    return query_;
  }

  auto
  fragment() const noexcept -> string_view
  {
    return fragment_;
  }

  auto
  is_http() const noexcept -> bool
  {
    return is_http_impl(scheme_);
  }

  auto
  is_authority() const noexcept -> bool
  {
    return scheme().empty() && !host().empty() && path().empty() && query().empty() &&
           fragment().empty();
  }

  auto
  is_absolute() const noexcept -> bool
  {
    return !scheme().empty() && fragment().empty();
  }
};

} // namespace foxy
