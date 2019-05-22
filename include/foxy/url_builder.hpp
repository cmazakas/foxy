#ifndef FOXY_URL_BUILDER_HPP_
#define FOXY_URL_BUILDER_HPP_

#include <foxy/uri.hpp>

#include <boost/utility/string_view.hpp>
#include <boost/locale/utf.hpp>

namespace foxy
{
namespace detail
{
template <class OutputIterator>
auto
encode_host(boost::u32string_view const host, OutputIterator out) -> OutputIterator
{
  using namespace foxy::uri::unicode;

  namespace x3  = boost::spirit::x3;
  namespace utf = boost::locale::utf;

  auto pos = host.begin();

  auto match = x3::parse(pos, host.end(), ip_literal | ip_v4_address);
  if (match) {
    for (auto const code_point : host) { out = utf::utf_traits<char>::encode(code_point, out); }
    return out;
  }

  pos = host.begin();
  while (pos < host.end()) {
    auto const old = pos;

    if (x3::parse(pos, host.end(), pct_encoded | sub_delims)) {
      for (auto begin = old; begin < pos; ++begin) {
        out = utf::utf_traits<char>::encode(*begin, out);
      }
      continue;
    }

    pos = old;
    ++pos;
  }

  return out;
}
} // namespace detail
} // namespace foxy

#endif // FOXY_URL_BUILDER_HPP_
