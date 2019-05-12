//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/uri.hpp>

#include <vector>
#include <iostream>

// Foxy offers up a direct alias for Boost.Spirit.X3 under its `uri` namespace
//
namespace x3 = foxy::uri::x3;

// the syntax of our command-line IP address parser will be:
// ip-parser --addresses=IP1,IP2,IP3,IP4
//
// clang-format off
//
// example correct input:
//
// ip-parser --addresses=127.0.0.1,FE80:0000:0000:0000:903A:1C1A:E802:11E4,FE80:0:0:0:903A::11E4,255.255.255.0
//
// These are all the IP addresses you entered:
// 127.0.0.1
// FE80:0000:0000:0000:903A:1C1A:E802:11E4
// FE80:0:0:0:903A::11E4
// 255.255.255.0
//
// clang-format on
//
int
main(int argc, char *argv[])
{
  if (argc != 2) { std::cout << "Incorrect number of command-line arguments!\n"; }

  auto const grammar =
    "--addresses=" >> (foxy::uri::raw[(foxy::uri::ip_v4_address | foxy::uri::ip_v6_address)] % ",");

  auto ip_addrs = std::vector<boost::string_view>();

  auto cli_arg = boost::string_view(argv[1], strlen(argv[1]));
  auto begin   = cli_arg.begin();

  auto const match = x3::parse(begin, cli_arg.end(), grammar, ip_addrs);

  if (!match) {
    std::cout << "An invalid IP address was detected!\n";
    std::cout << "Stopped traversal at:\n";
    std::cout << cli_arg.substr(begin - cli_arg.begin()) << "\n";
    return -1;
  }

  std::cout << "These are all the IP addresses you entered:\n";
  for (auto const ip_addr : ip_addrs) { std::cout << ip_addr << "\n"; }

  return 0;
}
