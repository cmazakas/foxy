//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/log.hpp>
#include <boost/asio/ssl/error.hpp>
#include <iostream>

namespace net = boost::asio;

auto
foxy::log_error(boost::system::error_code const ec, boost::string_view const what) -> void
{
  // don't bother printing ssl short reads
  // if an SSL stream truly _is_ truncated, Beast's parser errors out with a partial_message
  //
  if (ec == net::ssl::error::stream_truncated) { return; }
  std::cerr << what << " : " << ec.message() << "\n";
}
