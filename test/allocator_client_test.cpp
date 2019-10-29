//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/client_session.hpp>

#include <boost/beast/http.hpp>
#include <boost/beast/core/multi_buffer.hpp>

#include <boost/container/pmr/polymorphic_allocator.hpp>
#include <boost/container/pmr/monotonic_buffer_resource.hpp>

#include <memory>

#include <catch2/catch.hpp>

#include <iostream>

using boost::system::error_code;
using boost::asio::ip::tcp;

namespace asio = boost::asio;
namespace http = boost::beast::http;
namespace pmr  = boost::container::pmr;

using alloc_type   = pmr::polymorphic_allocator<char>;
using fields_type  = http::basic_fields<alloc_type>;
using body_type    = http::basic_string_body<char, std::char_traits<char>, alloc_type>;
using parser_type  = http::response_parser<body_type, alloc_type>;
using request_type = http::request<http::empty_body, fields_type>;

using client_type = foxy::basic_client_session<boost::beast::basic_multi_buffer<alloc_type>>;

namespace
{
#include <boost/asio/yield.hpp>
struct client_op : asio::coroutine
{
  using allocator_type = pmr::polymorphic_allocator<char>;

  client_type&  client;
  request_type& request;
  parser_type&  parser;
  bool&         was_valid;

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
    , alloc(std::move(alloc_))
  {
  }

  auto
  get_allocator() const noexcept -> allocator_type
  {
    return alloc;
  }

  auto
  operator()(error_code ec, tcp::endpoint) -> void
  {
    (*this)(ec);
  }

  auto operator()(error_code ec = {}) -> void
  {
    reenter(this)
    {
      yield client.async_connect("www.google.com", "80", std::move(*this));
      if (ec) {
        was_valid = false;
        yield break;
      }

      yield client.async_request(request, parser, std::move(*this));
      if (ec) {
        was_valid = false;
        yield break;
      }

      yield client.async_shutdown(std::move(*this));

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

TEST_CASE("allocator_client_test")
{
  SECTION("Allocator-aware client GET")
  {
    bool was_valid = false;

    asio::io_context io(1);

    auto const page_size = std::size_t{1024 * 512};

    pmr::monotonic_buffer_resource resource{page_size};

    auto alloc_handle = pmr::polymorphic_allocator<char>(std::addressof(resource));

    auto client = client_type(io.get_executor(), {}, alloc_handle);

    auto request = request_type(http::request_header<fields_type>(alloc_handle));
    request.method(http::verb::get);
    request.target("/");
    request.version(11);
    request.set(http::field::host, "www.google.com");

    parser_type parser{http::response_header<fields_type>(alloc_handle), alloc_handle};

    auto async_op = client_op(client, request, parser, was_valid, alloc_handle);
    CHECK(async_op.get_allocator() == alloc_handle);

    asio::post(io, std::move(async_op));

    io.run();

    CHECK(was_valid);
    CHECK(resource.remaining_storage() < page_size);
    CHECK(client.buffer.get_allocator() == alloc_handle);
  }
}
