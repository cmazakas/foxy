//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_TYPE_TRAITS_HPP_
#define FOXY_TYPE_TRAITS_HPP_

#include <boost/asio/async_result.hpp>
#include <boost/system/error_code.hpp>
#include <boost/type_traits/make_void.hpp>
#include <type_traits>

namespace foxy
{
namespace detail
{
template <class T, class = void>
struct is_closable_stream_throw : std::false_type
{
};

template <class T>
struct is_closable_stream_throw<T, boost::void_t<decltype(std::declval<T&>().close())>>
  : std::true_type
{
};

template <class T, class = void>
struct is_closable_stream_nothrow : std::false_type
{
};

template <class T>
struct is_closable_stream_nothrow<
  T,
  boost::void_t<decltype(std::declval<T&>().close(std::declval<boost::system::error_code&>()))>>
  : std::true_type
{
};

} // namespace detail
} // namespace foxy

#endif // FOXY_TYPE_TRAITS_HPP_
