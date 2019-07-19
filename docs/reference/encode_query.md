# foxy::uri::encode_query

## Include

```c++
#include <foxy/pct_encode.hpp>
```

## Declaration

```c++
template <class OutputIterator>
auto
encode_query(boost::u32string_view const query, OutputIterator out) -> OutputIterator;
```

## Synopsis

Writes the percent-encoded query component of a URI to the provided OutputIterator.

## Example

```c++
auto const query = boost::u32string_view(U"?//\":#/<>?@[\\]^`{|}");
auto       out   = std::string(256, '\0');

auto const encoded_query =
  boost::string_view(out.data(), foxy::uri::encode_query(query, out.begin()) - out.begin());

CHECK(encoded_query == "?//%22%3a%23/%3c%3e?%40%5b%5c%5d%5e%60%7b%7c%7d");
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
