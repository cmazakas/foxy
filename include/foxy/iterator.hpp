//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_ITERATOR_HPP_
#define FOXY_ITERATOR_HPP_

#include <boost/locale/utf.hpp>
#include <iterator>
#include <cstddef>

namespace foxy
{
namespace uri
{
template <class Iterator>
struct code_point_iterator
{
public:
  using value_type        = boost::locale::utf::code_point;
  using difference_type   = void;
  using pointer           = void;
  using reference         = value_type;
  using iterator_category = std::input_iterator_tag;

private:
  Iterator       iterator_;
  Iterator const end_;

public:
  code_point_iterator(void)                       = default;
  code_point_iterator(code_point_iterator const&) = default;
  code_point_iterator(code_point_iterator&&)      = default;

  code_point_iterator(Iterator const iterator, Iterator const end)
    : iterator_(iterator)
    , end_(end)
  {
  }

  auto operator*() -> value_type
  {
    return boost::locale::utf::utf_traits<std::iterator_traits<Iterator>::value_type>::decode(
      iterator_, end_);
  }

  auto
  operator++() -> code_point_iterator&
  {
    return *this;
  }

  auto
  operator++(int const) -> code_point_iterator&
  {
    return *this;
  }

  auto
  operator==(code_point_iterator const& rhs) -> bool
  {
    return iterator_ == rhs.iterator_;
  }

  auto
  operator!=(code_point_iterator const& rhs) -> bool
  {
    return !(*this == rhs);
  }
};

template <class Iterator>
auto
make_code_point_iterator(Iterator const iterator, Iterator const end)
  -> code_point_iterator<Iterator>
{
  return code_point_iterator<Iterator>(iterator, end);
}
} // namespace uri
} // namespace foxy

#endif // FOXY_ITERATOR_HPP_
