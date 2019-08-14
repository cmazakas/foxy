#include <foxy/uri_parts.hpp>

extern "C"
int
LLVMFuzzerTestOneInput(char const* data, size_t size)
{
  auto const input = boost::string_view(data, size);
  foxy::parse_uri(input);
  return 0;  // Non-zero return values are reserved for future use.
}
