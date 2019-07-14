# foxy::uri::encode_host

## Include

```c++
#include <foxy/pct_encode.hpp>
```

## Declaration

```c++
namespace uri
{
template <class OutputIterator>
auto
encode_host(boost::u32string_view const host, OutputIterator out) -> OutputIterator;
}
```

## Synopsis

Writes the percent-encoded host component of a URI to the provided OutputIterator.

## Example

```c++
auto const host = boost::u32string_view(U"hello world!\n");
auto       out  = std::string(256, '\0');

auto const encoded_host =
  boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

CHECK(encoded_host == "hello%20world!%0a");
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
