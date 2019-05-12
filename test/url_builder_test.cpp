#include <foxy/url_builder.hpp>
#include <foxy/iterator.hpp>
#include <foxy/uri.hpp>

#include <boost/utility/string_view.hpp>

#include <string>

#include <catch2/catch.hpp>

TEST_CASE("Our URL builder class...")
{
  // SECTION("should properly encode the host portion of a URL")
  // {
  //   auto const host = boost::string_view("hello world!\n");

  //   auto out = std::string(256, '\0');

  //   auto const end = foxy::detail::encode_host(foxy::uri::code_point_view<char>(host),
  //   out.begin());

  //   auto const encoded_host = boost::string_view(out.data(), end - out.begin());

  //   CHECK(encoded_host == "hello%20world\n");
  // }
}
