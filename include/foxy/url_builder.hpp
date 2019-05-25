#ifndef FOXY_URL_BUILDER_HPP_
#define FOXY_URL_BUILDER_HPP_

#include <foxy/uri.hpp>
#include <foxy/pct_encode.hpp>

#include <boost/utility/string_view.hpp>

#include <boost/locale/utf.hpp>

#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_string.hpp>
#include <boost/spirit/include/karma_sequence.hpp>
#include <boost/spirit/include/karma_numeric.hpp>
#include <boost/spirit/include/karma_right_alignment.hpp>

#include <array>

namespace foxy
{
namespace detail
{
template <class OutputIterator>
auto
encode_host(boost::u32string_view const host, OutputIterator out) -> OutputIterator
{
  using namespace foxy::uri::unicode;

  namespace x3    = boost::spirit::x3;
  namespace utf   = boost::locale::utf;
  namespace karma = boost::spirit::karma;

  auto pos = host.begin();

  auto match = x3::parse(pos, host.end(), ip_literal | ip_v4_address);
  if (match) {
    for (auto const code_point : host) { out = utf::utf_traits<char>::encode(code_point, out); }
    return out;
  }

  for (auto const code_point : host) {
    pos = host.begin();

    // no need to encode the normal ascii set
    //
    if ((code_point > 32) && (code_point < 127) &&
        boost::u32string_view(U"\"#/<>?@[\\]^`{|}").find(code_point) ==
          boost::u32string_view::npos) {
      out = utf::utf_traits<char>::encode(code_point, out);
      continue;
    }

    auto buffer = std::array<std::uint8_t, 4>{0xff, 0xff, 0xff, 0xff};

    auto const end = ::foxy::uri::utf8_encode(code_point, buffer.begin());

    for (auto pos = buffer.begin(); pos < end; ++pos) {
      karma::generate(out, karma::lit("%") << karma::right_align(2, karma::lit("0"))[karma::hex],
                      *pos);
    }
  }

  return out;
}

template <class OutputIterator>
auto
encode_path(boost::u32string_view const host, OutputIterator out) -> OutputIterator
{
  using namespace foxy::uri::unicode;

  namespace x3    = boost::spirit::x3;
  namespace utf   = boost::locale::utf;
  namespace karma = boost::spirit::karma;

  auto pos = host.begin();

  for (auto const code_point : host) {
    pos = host.begin();

    // no need to encode the normal ascii set
    //
    if ((code_point > 32) && (code_point < 127) &&
        boost::u32string_view(U"\"#<>?[\\]^`{|}").find(code_point) == boost::u32string_view::npos) {
      out = utf::utf_traits<char>::encode(code_point, out);
      continue;
    }

    auto buffer = std::array<std::uint8_t, 4>{0xff, 0xff, 0xff, 0xff};

    auto const end = ::foxy::uri::utf8_encode(code_point, buffer.begin());

    for (auto pos = buffer.begin(); pos < end; ++pos) {
      karma::generate(out, karma::lit("%") << karma::right_align(2, karma::lit("0"))[karma::hex],
                      *pos);
    }
  }

  return out;
}
} // namespace detail
} // namespace foxy

#endif // FOXY_URL_BUILDER_HPP_
