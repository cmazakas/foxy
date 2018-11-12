// // github.com/LeonineKing1199/foxy
// Copyright (c) 2018-2018 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/client_session.hpp>

foxy::client_session::client_session(boost::asio::io_context& io, session_opts opts)
: session(io, std::move(opts))
{
}
