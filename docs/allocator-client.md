# Allocator-Aware Client

One of the biggest draws of Asio is its support for Allocators.

Foxy tries to play up to this by allowing users to customize the allocator type used by the session
object's buffers.

In this example, we will focus on writing an allocator-aware client request to a well-known URL.

```c++
#include <foxy/client_session.hpp>

#include <boost/beast/http.hpp>

// we'll be using Beasts' basic_multi_buffer which is a buffer type that spans multiple independent
// ones in a series
// this will play better with our choice of allocator which can't afford costly reallocations
//
#include <boost/beast/core/multi_buffer.hpp>

// Boost.Container is our current source for our polymorphic allocator type
//
#include <boost/container/pmr/polymorphic_allocator.hpp>

// we'll be using the monotonic buffer resource which is a "dumb" allocator type that simply
// allocates by incrementing a buffer pointer
// the memory resource will also call out to its upstream once the current buffer has been exhaused
//
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#include <memory>
#include <iostream>

using boost::system::error_code;
using boost::asio::ip::tcp;

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace pmr  = boost::container::pmr;

using alloc_type   = pmr::polymorphic_allocator<char>;
using client_type  = foxy::client_session<boost::beast::basic_multi_buffer<alloc_type>>;
using body_type    = http::basic_string_body<char, std::char_traits<char>, alloc_type>;
using parser_type  = http::response_parser<body_type, alloc_type>;
using request_type = http::request<http::empty_body, http::basic_fields<alloc_type>>;

namespace
{
// we'll be propagating our allocator to Asio by using the allocator hooks
// this means that we need a callable object with a get_allocator method and a nested typedef of
// allocator_type
//
// to this end, Asio's stackless coroutine will suffice
//
// we include the corresponding yield and unyield headers to bring in the faux-keywords that Asio
// provides
//
#include <boost/asio/yield.hpp>
struct client_op : asio::coroutine
{
  using allocator_type = pmr::polymorphic_allocator<char>;

  // for the sake of simplicity, our coroutine will own none of the data it will touch
  //
  client_type&  client;
  request_type& request;
  parser_type&  parser;
  bool&         was_valid;

  // well, except for this allocator which can safely be copied by-value
  //
  allocator_type alloc;

  client_op()                     = delete;
  client_op(client_op const&)     = default;
  client_op(client_op&&) noexcept = default;

  client_op(client_type&   client_,
            request_type&  request_,
            parser_type&   parser_,
            bool&          was_valid_,
            allocator_type alloc_)
    : client(client_)
    , request(request_)
    , parser(parser_)
    , was_valid(was_valid_)
    , alloc(std::move(alloc_))
  {
  }

  auto
  get_allocator() const noexcept -> allocator_type
  {
    return alloc;
  }

  // keep this overload here so that our async_connect call has a way of reentering the coroutine
  //
  auto
  operator()(error_code ec, tcp::endpoint) -> void
  {
    (*this)(ec);
  }

  // our coroutine's main functionality
  //
  auto operator()(error_code ec = {}) -> void
  {
    reenter(this)
    {
      yield client.async_connect("www.google.com", "80", *this);
      if (ec) {
        was_valid = false;
        yield break;
      }

      yield client.async_request(request, parser, *this);
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

  // run the io_context with a single-thread in mind
  // this allows Asio to make certain assumptions it can optimize around
  //
  asio::io_context io(1);

  // this is intial size of the buffer that we'll be using to send our request
  //
  auto const page_size = std::size_t{1024 * 512};

  // create the resource and grab a handle to it via the polymorphic allocators
  // these allocators enable handles to stateful memory resources to be copied cheaply
  //
  pmr::monotonic_buffer_resource resource{page_size};

  auto alloc_handle = pmr::polymorphic_allocator<char>(std::addressof(resource));

  // generate our client session, making sure to provide a handle to our memory resource
  //
  auto client  = client_type(io, {}, alloc_handle);

  // build up our request object
  // even though our request has an empty body type, we still need to pass a handle to the memory
  // resource
  // this is because setting fields in Beast requires an allocation
  //
  auto request = request_type(http::request_header<http::basic_fields<alloc_type>>(alloc_handle));
  request.method(http::verb::get);
  request.target("/");
  request.version(11);
  request.set(http::field::host, "www.google.com");

  // we also build up our response parser type
  // again, we must pass in an instance of a header that's been instantiead with an allocator to
  // propagate it properly
  // the second copy of the allocator is forwarded to the body type of the parser (in this case,
  // an allocator-aware string body)
  //
  parser_type parser{http::response_header<http::basic_fields<alloc_type>>(alloc_handle),
                     alloc_handle};

  // we post our coroutine to the io_context for processing
  //
  asio::post(io, client_op{client, request, parser, was_valid, alloc_handle});

  io.run();

  auto res = parser.release();
  std::cout << "Response received from google:\n";
  std::cout << res.body() << "\n";

  std::cout << "We used this many bytes to read in this message: "
            << page_size - resource.remaining_storage() << "\n";

  return 0;
}
```

To [ToC](./index.md#Table-of-Contents)
