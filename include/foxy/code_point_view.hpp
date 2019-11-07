//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_CODE_POINT_VIEW_HPP_
#define FOXY_CODE_POINT_VIEW_HPP_

#include <foxy/code_point_iterator.hpp>

namespace foxy
{
template <class Char, class Traits = std::char_traits<Char>>
struct code_point_view
{
public:
  using iterator_type =
    code_point_iterator<typename boost::basic_string_view<Char, Traits>::iterator>;

private:
  boost::basic_string_view<Char, Traits> view_;

public:
  code_point_view()                       = default;
  code_point_view(code_point_view const&) = default;
  code_point_view(code_point_view&&)      = default;

  code_point_view(boost::basic_string_view<Char, Traits> view)
    : view_(view)
  {
  }

  auto
  begin() const noexcept -> iterator_type
  {
    return make_code_point_iterator(view_.begin(), view_.end());
  }

  auto
  end() const noexcept -> iterator_type
  {
    return make_code_point_iterator(view_.end(), view_.end());
  }
};
} // namespace foxy

#endif // FOXY_CODE_POINT_VIEW_HPP_
