#include <foxy/uri_parts.hpp>

#include <vector>
#include <cstring>

extern "C" int
LLVMFuzzerTestOneInput(char const* data, size_t const size)
{
  auto const input = boost::string_view(data, size);
  foxy::parse_uri(input);

  auto const num_code_points = input.size() / 4;
  auto       code_points     = std::vector<char32_t>(num_code_points);

  std::memcpy(code_points.data(), input.data(), num_code_points * 4);

  auto const unicode_input = boost::u32string_view(code_points.data(), code_points.size());
  foxy::parse_uri(unicode_input);

  return 0; // Non-zero return values are reserved for future use.
}
