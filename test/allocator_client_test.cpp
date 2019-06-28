//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/client_session.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/multi_buffer.hpp>

#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#include <memory>
#include <iostream>

#include <catch2/catch.hpp>

using boost::system::error_code;
using boost::asio::ip::tcp;

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace pmr  = boost::container::pmr;

using allocator_type = pmr::polymorphic_allocator<char>;
using client_type    = foxy::client_session<boost::beast::basic_multi_buffer<allocator_type>>;
using body_type      = http::basic_string_body<char, std::char_traits<char>, allocator_type>;
using parser_type    = http::response_parser<body_type, allocator_type>;
using request_type   = http::request<http::empty_body>;

namespace
{
#include <boost/asio/yield.hpp>
struct client_op : asio::coroutine
{
  client_type&  client;
  request_type& request;
  parser_type&  parser;
  bool&         was_valid;

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

  using allocator_type = pmr::polymorphic_allocator<char>;

  auto
  get_allocator() const noexcept -> allocator_type
  {
    return alloc;
  }

  auto
  operator()(boost::system::error_code ec, boost::asio::ip::tcp::endpoint) -> void
  {
    (*this)(ec);
  }

  auto operator()(boost::system::error_code ec = {}) -> void
  {
    reenter(this)
    {
      yield client.async_connect("www.google.com", "80", *this);
      std::cout << "connected to the client!!!\n";

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
        auto response = parser.release();
        was_valid     = (response.result_int() == 200 && response.body().size() > 0 &&
                     boost::string_view(response.body()).ends_with("</html>"));
      }
    }
  }
};
#include <boost/asio/unyield.hpp>
} // namespace

TEST_CASE("Our allocator-aware client session class")
{
  SECTION("... should be able to do a GET with 1 system allocation")
  {
    bool was_valid = false;

    asio::io_context io(1);

    auto const page_size = std::size_t{10 * 1024 * 1024};

    pmr::monotonic_buffer_resource resource{page_size};

    auto alloc_handle = pmr::polymorphic_allocator<char>(std::addressof(resource));

    auto client  = client_type(io, {}, alloc_handle);
    auto request = http::request<http::empty_body>(http::verb::get, "/", 11);

    parser_type parser{http::response_header<http::basic_fields<allocator_type>>(alloc_handle),
                       alloc_handle};

    asio::post(io, client_op{client, request, parser, was_valid, alloc_handle});

    io.run();

    REQUIRE(was_valid);
    REQUIRE(page_size > resource.remaining_storage());
  }
}
