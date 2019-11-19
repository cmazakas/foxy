//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#define CATCH_CONFIG_MAIN

#include <foxy/session.hpp>
#include <foxy/detail/timed_op_wrapper_v3.hpp>

#include <boost/asio/io_context.hpp>

#include <catch2/catch.hpp>

namespace asio = boost::asio;

TEST_CASE("timed_op_wrapper_v3")
{
  asio::io_context io{1};

  auto session = foxy::session(io, foxy::session_opts{});

  auto impl = [&session](auto& self, boost::system::error_code ec = {},
                         std::size_t bytes_transferred = 0) mutable { self.complete({}, 0); };

  foxy::detail::async_timer<void(boost::system::error_code ec, std::size_t bytes_transferred)>(
    std::move(impl), session, [](boost::system::error_code ec, std::size_t bytes_transferred) {});

  io.run();
}
