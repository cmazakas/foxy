#include <foxy/uri_parts.hpp>

#include <vector>
#include <cstring>

extern "C" int
LLVMFuzzerTestOneInput(char const* data, size_t const size)
{
  auto const input = boost::string_view(data, size);
  foxy::parse_uri(input);

  auto code_points = std::vector<char32_t>(size);
  std::memcpy(code_points.data(), input.data(), size);

  auto const unicode_input = boost::u32string_view(code_points.data(), code_points.size());
  foxy::parse_uri(input);

  return 0; // Non-zero return values are reserved for future use.
}
