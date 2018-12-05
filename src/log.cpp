//
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot
// com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/log.hpp>

auto
foxy::log_error(boost::system::error_code const ec,
                boost::string_view const        what) -> void
{
  std::cerr << what << " : " << ec.message() << "\n";
}
