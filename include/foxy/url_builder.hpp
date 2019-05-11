#ifndef FOXY_URL_BUILDER_HPP_
#define FOXY_URL_BUILDER_HPP_

namespace foxy
{
namespace detail
{
template <class InputRange, class OutputIterator>
auto
encode_host(InputRange codepoint_range, OutputIterator out) -> OutputIterator
{
  return out;
}
} // namespace detail
} // namespace foxy

#endif // FOXY_URL_BUILDER_HPP_
