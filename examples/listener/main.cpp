#include <foxy/listener.hpp>
#include <foxy/client_session.hpp>
#include <foxy/pct_encode.hpp>
#include <foxy/code_point_iterator.hpp>

// we use this to setup our SSL contexts for the server and client
// these shouldn't really be considered production-quality but for the sake of an example, they'll
// have to do
//
#include <foxy/test/helpers/ssl_ctx.hpp>

#include <boost/asio/spawn.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/coroutine.hpp>

#include <boost/beast/http.hpp>

#include <iostream>
#include <memory>
#include <string>

#include <boost/asio/yield.hpp>

namespace asio = boost::asio;
namespace ip   = boost::asio::ip;
namespace http = boost::beast::http;

using tcp = boost::asio::ip::tcp;

// this examples creates a simple google query proxy kind of server
// users send their query text as a message body and the server will URL encode it and send the
// request to google as a general query one would normally get by interacting directly with
// www.google.com in their browser
//
int
main()
{
  asio::io_context io{1};

  // create the HTTP server, listening at 127.0.0.1:1337
  //
  auto listener =
    foxy::listener(io.get_executor(),
                   tcp::endpoint(ip::make_address("127.0.0.1"), static_cast<unsigned short>(1337)));

  // begin the acceptance loop
  //
  // we capture the `io_context` by reference so that we can spawn new client session's over its
  // executor
  //
  // our factory function also uses a local static client SSL context for our client to use
  // because this is a static, it is only initialized the first time
  //
  listener.async_accept([&io](auto& server) {
    static auto client_ssl_ctx = foxy::test::make_client_ssl_ctx();

    return
      //
      // in this case, our request handler is going to be a stackless coroutine written using a
      // lambda
      //
      // we use the lambda captures to create the coroutine frame
      // we need some form of address stability for our async operations so everything must be
      // heap-allocated
      // we need a request + response for our server to interact with its client and we also need to
      // allocate our client session and then client request/response for when we forward the
      // client's body as a google query we can also embed the state of the coroutine into the
      // lambda capture as well
      //
      [&server, coro = asio::coroutine(),
       request  = std::make_unique<http::request<http::string_body>>(),
       response = std::make_unique<http::response<http::string_body>>(),
       client   = std::make_unique<foxy::client_session>(
         io.get_executor(), foxy::session_opts{client_ssl_ctx, std::chrono::seconds{30}}),
       client_req = std::make_unique<http::request<http::empty_body>>(),
       client_res = std::make_unique<http::response<http::string_body>>()](
        //
        // `auto& self` is required by `asio::async_compose`
        // the user supplies a callable that takes a mutable reference to a composed_op wrapper that
        // wraps their supplied callable
        // note, `std::move`ing `self` will also move the surrounding callable
        // this can lead to undefined behavior if the user accesses a variable in the frame and also
        // calls something like `boost::beast::bind_handler(std::move(self), ...)`
        // another restriction is that supplied callable must be invocable as just `impl(self)` so
        // we use defaulted-arguments to enable this
        //
        auto& self, boost::system::error_code ec = {}, std::size_t bytes_transferred = 0) mutable {
        reenter(coro)
        {
          // our server will have a timeout of 30 seconds for its read/write operations
          //
          server.opts.timeout = std::chrono::seconds{30};

          // read in the client reuest
          //
          yield server.async_read(*request, std::move(self));
          if (ec) { return self.complete(ec, bytes_transferred); }

          // if the request verb was not POST, respond with an error and terminate the server
          // session
          //
          if (request->method() != http::verb::post) {
            response->result(http::status::method_not_allowed);
            response->body() = "The server only supports POST requests!\n";
            response->prepare_payload();

            yield server.async_write(*response, std::move(self));
            return self.complete(ec, bytes_transferred);
          }

          // if the supplied message body was empty, respond back with an ill-formed body status
          // the client is required to send a non-zero length query
          //
          if (request->body().size() == 0) {
            response->result(http::status::bad_request);
            response->body() = "The client body must not be empty!\n";
            response->prepare_payload();

            yield server.async_write(*response, std::move(self));
            return self.complete(ec, bytes_transferred);
          }

          // now we prepare the client request
          // we set our method to GET and add the appropriate Host header field as is required by
          // HTTP/1.1
          //
          client_req->method(http::verb::get);
          client_req->set(http::field::host, "www.google.com");
          {
            // we use Foxy's methods to url encode the message body as a query string
            // we begin by creating a "sink" which will store the output of our operations
            // in this case, a simple `std::string` that starts with the request target and the
            // initial ? for query params
            //
            auto target = std::string("/webhp?");

            // create a code_point_iterator pair via the code_point_view
            // persist the view to storage by using `std::vector`'s iterator-based constructor
            // using this, we can feed it to encode_query will convert the range of char32_t values
            // into proper percent-encoded utf-8
            // we use `std::back_inserter(target)` so that the output is appended to our request
            // target
            //
            auto const view        = foxy::code_point_view<char>(request->body());
            auto const code_points = std::vector<char32_t>(view.begin(), view.end());
            foxy::uri::encode_query({code_points.data(), code_points.size()},
                                    std::back_inserter(target));

            client_req->target(target);
          }

          // we now connect to google and then send the request
          //
          yield client->async_connect("www.google.com", "https", std::move(self));
          yield client->async_request(*client_req, *client_res, std::move(self));
          if (ec) {
            response->result(http::status::bad_request);
            response->body() = "Error communicating with google!\n";
            response->prepare_payload();

            yield server.async_write(*response, std::move(self));
            yield client->async_shutdown(std::move(self));
            return self.complete(ec, bytes_transferred);
          }

          // forward the response from google back to the client directly
          //
          if (client_res->result() != http::status::ok) {
            response->result(http::status::bad_request);
            response->body() = "Error is: " + client_res->body() + "\n";
          } else {
            response->result(http::status::ok);
            response->body() = client_res->body();
          }
          response->prepare_payload();
          yield server.async_write(*response, std::move(self));

          // note, we only need to shutdown the client here
          // the `foxy::listener` handles the termination of server sessions automatically for the
          // user
          //
          yield client->async_shutdown(std::move(self));

          self.complete({}, 0);
        }
      };
  });

  // this is our client operation
  // a simple client that sends a request to our server with a body of "hello world in spanish"
  //
  asio::spawn(io.get_executor(), [&](auto yield_ctx) mutable {
    auto client = foxy::client_session(io.get_executor(), {{}, std::chrono::seconds(30), false});
    client.async_connect("127.0.0.1", "1337", yield_ctx);

    auto req = http::request<http::string_body>(http::verb::post, "/", 11);
    auto res = http::response<http::string_body>();

    req.body() = "hello world in spanish";
    req.prepare_payload();

    client.async_request(req, res, yield_ctx);

    std::cout << "Got the response:\n" << res << "\n";

    auto ec = boost::system::error_code();
    client.async_shutdown(yield_ctx[ec]);

    listener.shutdown();
  });

  io.run();

  return 0;
}
