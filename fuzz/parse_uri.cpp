#include <foxy/parse_uri.hpp>

#include <vector>
#include <cstring>
#include <algorithm>

extern "C" int
LLVMFuzzerTestOneInput(char const* data, size_t const size)
{
  auto const input = boost::string_view(data, size);
  foxy::parse_uri(input);

  auto const num_code_points = input.size() / sizeof(char32_t);
  if (num_code_points == 0) { return 0; }

  auto code_points = std::vector<char32_t>(num_code_points);
  std::memcpy(code_points.data(), input.data(), num_code_points * sizeof(char32_t));

  // manually filtering out these invalid code points will become obsolete with Boost 1.72
  // which fixes a bug in X3's Unicode parsing functions
  //
  auto const valid_size =
    std::remove_if(code_points.begin(), code_points.end(),
                   [](auto const code_point) -> bool { return code_point > 0x10ffff; }) -
    code_points.begin();

  auto const unicode_input = boost::u32string_view(code_points.data(), valid_size);
  foxy::parse_uri(unicode_input);

  return 0;
}
