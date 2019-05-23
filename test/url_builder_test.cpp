#include <foxy/url_builder.hpp>
#include <foxy/iterator.hpp>
#include <foxy/uri.hpp>

#include <boost/utility/string_view.hpp>

#include <string>

#include <catch2/catch.hpp>

TEST_CASE("Our URL builder class...")
{
  SECTION("should not encode pct-encoded and the sub-delims")
  {
    auto const host = boost::u32string_view(U"%20%13%24!$'()*+,;=");

    auto out = std::string(256, '\0');

    auto const end          = foxy::detail::encode_host(host, out.begin());
    auto const encoded_host = boost::string_view(out.data(), end - out.begin());

    CHECK(encoded_host == "%20%13%24!$'()*+,;=");
  }

  SECTION("should pct-encode misc. whitespace chars")
  {
    auto const host = boost::u32string_view(U"hello world!\n");
    auto       out  = std::string(256, '\0');

    auto const encoded_host =
      boost::string_view(out.data(), foxy::detail::encode_host(host, out.begin()) - out.begin());

    CHECK(encoded_host == "hello%20world!%0a");
  }
}
