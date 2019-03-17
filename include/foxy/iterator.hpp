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

#include <boost/utility/string_view.hpp>
#include <boost/locale/utf.hpp>
#include <iterator>
#include <utility>
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
  using difference_type   = std::ptrdiff_t;
  using pointer           = void;
  using reference         = value_type;
  using iterator_category = std::input_iterator_tag;

private:
  Iterator iterator_;
  Iterator end_;

public:
  code_point_iterator(void)                           = default;
  code_point_iterator(code_point_iterator const&)     = default;
  code_point_iterator(code_point_iterator&&) noexcept = default;

  code_point_iterator(Iterator const iterator, Iterator const end)
    : iterator_(iterator)
    , end_(end)
  {
  }

  auto
  operator=(code_point_iterator const& rhs) & -> code_point_iterator&
  {
    iterator_ = rhs.iterator_;
    end_      = rhs.end_;
    return *this;
  }

  auto
    operator=(code_point_iterator&& rhs) &
    noexcept -> code_point_iterator&
  {
    iterator_ = std::move(rhs.iterator_);
    end_      = std::move(rhs.end_);

    rhs.iterator_ = nullptr;
    rhs.end_      = nullptr;

    return *this;
  }

  auto operator*() -> value_type
  {
    return boost::locale::utf::utf_traits<std::iterator_traits<Iterator>::value_type>::decode(
      iterator_, end_);
  }

  auto
  operator++() & -> code_point_iterator&
  {
    return *this;
  }

  auto
  operator++(int const) & -> code_point_iterator&
  {
    return *this;
  }

  auto
  operator==(code_point_iterator const& rhs) const noexcept -> bool
  {
    return iterator_ == rhs.iterator_;
  }

  auto
  operator!=(code_point_iterator const& rhs) const noexcept -> bool
  {
    return !(*this == rhs);
  }

  auto
  swap(code_point_iterator& rhs) -> void
  {
    auto tmp_iterator = rhs.iterator_;
    auto tmp_end      = rhs.end_;

    rhs.iterator_ = iterator_;
    rhs.end_      = end_;

    iterator_ = tmp_iterator;
    end_      = tmp_end;
  }
};

namespace code_point
{
template <class Iterator>
auto
swap(code_point_iterator<Iterator>& iter1, code_point_iterator<Iterator>& iter2)
{
  iter1.swap(iter2);
}
} // namespace code_point

template <class Iterator>
auto
make_code_point_iterator(Iterator const iterator, Iterator const end)
  -> code_point_iterator<Iterator>
{
  return code_point_iterator<Iterator>(iterator, end);
}

template <class Char, class Traits = std::char_traits<Char>>
struct code_point_view
{
public:
  using iterator_type = typename boost::basic_string_view<Char, Traits>::iterator;

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
  begin() const noexcept -> code_point_iterator<iterator_type>
  {
    return make_code_point_iterator(view_.begin(), view_.end());
  }

  auto
  end() const noexcept -> code_point_iterator<iterator_type>
  {
    return make_code_point_iterator(view_.end(), view_.end());
  }
};

} // namespace uri
} // namespace foxy

#endif // FOXY_ITERATOR_HPP_
