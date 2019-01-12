//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
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
#include <type_traits>

namespace foxy
{
template <class... Ts>
struct make_void
{
  typedef void type;
};
template <class... Ts>
using void_t = typename make_void<Ts...>::type;

template <class CompletionToken, class Signature>
using return_t =
  typename boost::asio::async_result<std::decay_t<CompletionToken>, Signature>::return_type;

template <class CompletionToken, class Signature>
using completion_handler_t =
  typename boost::asio::async_completion<CompletionToken, Signature>::completion_handler_type;

namespace detail
{
template <class T, class = void>
struct is_closable_stream_throw : std::false_type
{
};

template <class T>
struct is_closable_stream_throw<T, void_t<decltype(std::declval<T&>().close())>> : std::true_type
{
};

template <class T, class = void>
struct is_closable_stream_nothrow : std::false_type
{
};

template <class T>
struct is_closable_stream_nothrow<
  T,
  void_t<decltype(std::declval<T&>().close(std::declval<boost::system::error_code&>()))>>
  : std::true_type
{
};

} // namespace detail
} // namespace foxy

#endif // FOXY_TYPE_TRAITS_HPP_
