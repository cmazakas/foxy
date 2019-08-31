//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/client_session.hpp>

#include <boost/beast/http/message.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>

#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>

#include <iostream>

namespace asio = boost::asio;
namespace http = boost::beast::http;

int
main()
{
  // this is the main workhorse for Asio and effectively operates the same as the event loop in
  // Node.js
  //
  asio::io_context io{1};

  // we now construct a `client_session` with the default parameters (no SSL, timeout of 1s on
  // read/write ops)
  //
  auto client = foxy::client_session(io.get_executor(), {});

  // we now choose to spin up a "stackful" coroutine
  // this essentially means that this coroutine suspends by taking a snapshot of the current call
  // stack and any registers and persists them to some allocated storage
  //
  // the coroutine resumes by copying the stack back and setting all the appropriate registers again
  //
  // this effectively enables the coroutine to be suspended from below us as the actual suspension
  // happens in our underlying function calls
  //
  asio::spawn(io.get_executor(), [&](asio::yield_context yield) {
    // these arguments get forwarded directly to Asio via Foxy
    // `host` is somewhat self-explanatory but `service` can be either a port number directly or a
    // protocol as is demonstrated here
    //
    auto const* const host    = "www.google.com";
    auto const* const service = "http";

    auto const http_version = 11; // 1.1

    // we build our message containers up-front
    // `request` is a simple "GET / HTTP/1.1\r\n\r\n"
    // `response` is declared up-front as the intended container type for the reply from Google
    //
    auto request  = http::request<http::empty_body>(http::verb::get, "/", http_version);
    auto response = http::response<http::string_body>();

    // we now attempt to connect to google.com, using plain HTTP
    // note, if there is an error, this coroutine will be resumed with an exception
    //
    client.async_connect(host, service, yield);

    // we now write the request and read the response
    //
    client.async_request(request, response, yield);

    // closing a connection is either synchronous or asynchronous depending on whether or not the
    // underlying stream is encrypted or not
    // Foxy doesn't directly handle this so it's up to the user to gracefully close the connection
    // with the connected remote
    //
    // we access the plain portion of the wrapped multi-stream and do normal Asio TCP connection
    // closing
    //
    auto& socket = client.stream.plain();
    socket.shutdown(asio::ip::tcp::socket::shutdown_both);
    socket.close();

    std::cout << "Got a response back from Google!\n";
    std::cout << response << "\n\n";

    // at this stage, we've done everything we needed to!
    // we turned the human-readable domain name into a set of IPs, formed a TCP connection with the
    // first one we could and then wrote a request object to the underlying TCP stream and read the
    // response back in a container that we control ourselves
    // we finish off with a bit of manual work, gracefully closing the TCP connection ourselves
    //
  });

  // we've created the code for our coroutine but we haven't actually done any work yet
  // we must now register the main thread as a worker thread for Asio's `io_context`
  // `io_context::run` will keep the calling thread busy until there's no more work to do
  // in this case, we have only one submitted task right now, our above coroutine
  //
  io.run();

  return 0;
}
