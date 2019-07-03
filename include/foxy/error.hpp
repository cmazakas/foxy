//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_ERROR_HPP_
#define FOXY_ERROR_HPP_

#include <boost/system/error_code.hpp>
#include <type_traits>

namespace foxy
{
enum class error {
  // our pct-decoding function hit a consecutive percent sign which means it doesn't follow a valid
  // grammar for URIs
  //
  unexpected_pct = 1
};
}

namespace boost
{
namespace system
{
template <>
struct is_error_code_enum<::foxy::error>
{
  static bool const value = true;
};

} // namespace system
} // namespace boost

namespace foxy
{
namespace detail
{
class foxy_error_category : public boost::system::error_category {
public:
  const char*
  name() const noexcept override
  {
    return "foxy";
  }

  std::string
  message(int ev) const override
  {
    switch (static_cast<::foxy::error>(ev)) {
      case ::foxy::error::unexpected_pct: return "consecutive percent sign detected";

      default: return "foxy default error";
    }
  }

  boost::system::error_condition
  default_error_condition(int ev) const noexcept override
  {
    return boost::system::error_condition{ev, *this};
  }

  bool
  equivalent(int ev, boost::system::error_condition const& condition) const noexcept override
  {
    return condition.value() == ev && &condition.category() == this;
  }

  bool
  equivalent(boost::system::error_code const& error, int ev) const noexcept override
  {
    return error.value() == ev && &error.category() == this;
  }
};
} // namespace detail

auto
make_error_code(::foxy::error ev) -> boost::system::error_code
{
  static detail::foxy_error_category const cat{};
  return boost::system::error_code{static_cast<std::underlying_type<::foxy::error>::type>(ev), cat};
}

} // namespace foxy

#endif // FOXY_ERROR_HPP_
