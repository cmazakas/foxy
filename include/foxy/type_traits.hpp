//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#ifndef FOXY_TYPE_TRAITS_HPP_
#define FOXY_TYPE_TRAITS_HPP_

#include <boost/system/error_code.hpp>
#include <type_traits>

namespace foxy
{

template <class ...Ts> struct make_void { typedef void type;};
template <class ...Ts> using void_t = typename make_void<Ts...>::type;

namespace detail
{

template <class T, class = void>
struct is_closable_stream_throw : std::false_type {};

template <class T>
struct is_closable_stream_throw<
  T, void_t<
    decltype(std::declval<T&>().close())
  >
> : std::true_type {};

template <class T, class = void>
struct is_closable_stream_nothrow : std::false_type {};

template <class T>
struct is_closable_stream_nothrow<
  T, void_t<
    decltype(std::declval<T&>().close(std::declval<boost::system::error_code&>()))
  >
> : std::true_type {};

} // detail
} // foxy

#endif // FOXY_TYPE_TRAITS_HPP_
