# foxy::uri::encode_path

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
encode_path(boost::u32string_view const path, OutputIterator out) -> OutputIterator;
}
```

## Synopsis

Writes the percent-encoded path component of a URI to the provided OutputIterator.

## Example

```c++
auto const path = boost::u32string_view(U"/\"#<>?@[\\]^`{|}:::");
auto       out  = std::string(256, '\0');

auto const encoded_path =
  boost::string_view(out.data(), foxy::uri::encode_path(path, out.begin()) - out.begin());

CHECK(encoded_path == "/%22%23%3c%3e%3f@%5b%5c%5d%5e%60%7b%7c%7d:::");
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
