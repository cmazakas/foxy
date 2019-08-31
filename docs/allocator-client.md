# Allocator-Aware Client

One of the biggest draws of Asio is its support for Allocators.

Much like Beast, Foxy embraces this and ensures that if a user's completion handler has an
associated allocator that it is used for intermediate allocations by the initiating functions.

In this example, we will focus on writing an allocator-aware client request to a well-known URL.

```c++
#include <foxy/client_session.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/multi_buffer.hpp>

#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#include <memory>
#include <iostream>

using boost::system::error_code;
using boost::asio::ip::tcp;

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace pmr  = boost::container::pmr;

// Boost.Container ships with an implementation of the polymorphic allocators added to C++17
// https://www.boost.org/doc/libs/release/doc/html/container/cpp_conformance.html#container.cpp_conformance.polymorphic_memory_resources
//
// we will be using these polymorphic allocators to share the same memory resource among all of our
// async operations
//
using alloc_type = pmr::polymorphic_allocator<char>;

// our headers, body and response parser all need to share the same memory resource so they must all
// use the same allocator
//
using fields_type = http::basic_fields<alloc_type>;
using body_type   = http::basic_string_body<char, std::char_traits<char>, alloc_type>;
using parser_type = http::response_parser<body_type, alloc_type>;

// even though our request type has an empty body, we still need to make sure the allocator is
// propagated
//
// this is because internally Beast uses the allocator to create the header fields and we will need
// header fields to send a request
//
using request_type = http::request<http::empty_body, fields_type>;

// we use the basic_client_session over Beast's multi buffer type
// the multi_buffer is a Beast Body type that internally uses a sequence of buffers
//
using client_type = foxy::basic_client_session<boost::beast::basic_multi_buffer<alloc_type>>;

namespace
{
// to write an allocator-aware operation in Asio, we need a callable object that has a get_allocator
// member function and a nested allocator_type typedef
//
// to this end, an Asio stackless coroutine will suffice
//
// we bring in the yield/unyield headers to import the faux-keywords Asio adds
//
// these give us: yield, reenter
//
#include <boost/asio/yield.hpp>
struct client_op : asio::coroutine
{
  using allocator_type = pmr::polymorphic_allocator<char>;
  using executor_type  = typename client_type::executor_type;

  // our async operation will own none of its required data
  //
  client_type&  client;
  request_type& request;
  parser_type&  parser;
  bool&         was_valid;

  // because our allocator is essentially just a pointer to a resouce, it's cheap to copy and to
  // also pass by value
  //
  allocator_type alloc;

  client_op()                 = delete;
  client_op(client_op const&) = default;
  client_op(client_op&&)      = default;

  client_op(client_type&   client_,
            request_type&  request_,
            parser_type&   parser_,
            bool&          was_valid_,
            allocator_type alloc_)
    : client(client_)
    , request(request_)
    , parser(parser_)
    , was_valid(was_valid_)
    , alloc(alloc_)
  {
  }

  auto
  get_allocator() const noexcept -> allocator_type
  {
    return alloc;
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return client.get_executor();
  }

  // this overload of operator() is to enable reentry from our async_connect function
  //
  auto
  operator()(error_code ec, tcp::endpoint) -> void
  {
    (*this)(ec);
  }

  // the entrypoint into our coroutine
  //
  auto operator()(error_code ec = {}) -> void
  {
    reenter(this)
    {
      yield client.async_connect("www.google.com", "80", std::move(*this));
      if (ec) {
        was_valid = false;

        // "yield break;" is how one terminates an Asio stackless coroutine early
        yield break;
      }

      yield client.async_request(request, parser, std::move(*this));
      if (ec) {
        was_valid = false;
        yield break;
      }

      {
        auto& response = parser.get();
        was_valid      = (response.result_int() == 200 && response.body().size() > 0 &&
                     boost::string_view(response.body()).ends_with("</html>"));
      }
    }
  }
};
#include <boost/asio/unyield.hpp>
} // namespace

int
main()
{
  bool was_valid = false;

  asio::io_context io{1};

  // we give our async op around half a MB up front
  //
  auto const page_size = std::size_t{1024 * 512};

  // we use the monotonic buffer resource which is a "dumb" allocator type that simply only
  // increments an offset from the start of a buffer for its allocations
  //
  // because allocation is just incrementing a buffer offset, allocations are cheap
  //
  pmr::monotonic_buffer_resource resource{page_size};

  auto alloc_handle = pmr::polymorphic_allocator<char>(std::addressof(resource));

  // we construct our client with a handle to the memory resource
  //
  auto client = client_type(io.get_executor(), {}, alloc_handle);

  // we create our request, making sure that our headers are constructed with a handle to the
  // resource as well
  //
  auto request = request_type(http::request_header<fields_type>(alloc_handle));
  request.method(http::verb::get);
  request.target("/");
  request.version(11);
  request.set(http::field::host, "www.google.com");

  // our parser's underlying message is comprised of 2 parts, its headers and body
  // this means we need to forward our allocator to both the header and the body separately
  // Beast's parser forwards arguments to the wrapped message and messages are directly
  // constructible with a header instance and any arguments we want to construct the body with
  //
  parser_type parser{http::response_header<fields_type>(alloc_handle), alloc_handle};

  // our client, request and response parser all now share the same memory resource
  // this means all intermediate allocations will be done using a simple pointer increment
  //

  // we create an instance of our coroutine and then post it the io_context for execution
  //
  auto async_op = client_op(client, request, parser, was_valid, alloc_handle);
  asio::post(io.get_executor(), std::move(async_op));

  io.run();

  auto res = parser.release();

  if (!was_valid) {
    std::cout << "error could not complete request successfully\n" << res << "\n";
    return 1;
  }

  std::cout << "Completed request!\n";
  std::cout << "-----------------------------------------------------------------\n";
  std::cout << res.base() << "\n";
  std::cout << "-----------------------------------------------------------------\n";
  std::cout << "remaining storage in buffer: " << resource.remaining_storage() << "\n\n";

  return 0;
}

```

---

To [ToC](./index.md#Table-of-Contents)
