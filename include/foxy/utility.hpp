//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/f3
//

#ifndef FOXY_UTILITY_HPP_
#define FOXY_UTILITY_HPP_

#include <boost/utility/string_view.hpp>
#include <utility>

namespace foxy
{

auto
parse_authority_form(boost::string_view const uri)
-> std::pair<std::string, std::string>;

} // foxy

#endif // FOXY_UTILITY_HPP_
