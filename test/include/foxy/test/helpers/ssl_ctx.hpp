//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <boost/asio/ssl/context.hpp>

namespace foxy
{
namespace test
{
auto
make_server_ssl_ctx() -> boost::asio::ssl::context;

auto
make_client_ssl_ctx() -> boost::asio::ssl::context;
} // namespace test
} // namespace foxy
