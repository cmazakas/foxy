# foxy::parse_uri

## Include

```c++
#include <foxy/parse_uri.hpp>
```

## Declaration

```c++
auto
parse_uri(boost::basic_string_view<char, std::char_traits<char>> const uri)
  -> basic_uri_parts<char>;

auto
parse_uri(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri)
  -> basic_uri_parts<char32_t>;
```

## Synopsis

Parse the string denoted by `uri`. If a valid URI has been supplied, this function will return a
non-empty [`basic_uri_parts`](./uri_parts.md#foxybasic_uri_parts). Otherwise, a
default-constructed instance is returned.

## Example

```c++
auto const view      = boost::u32string_view(U"http://www.google.com:80/hello?query#fragment");
auto const uri_parts = foxy::parse_uri(view);

CHECK(uri_parts.scheme() == U"http");
CHECK(uri_parts.host() == U"www.google.com");
CHECK(uri_parts.port() == U"80");
CHECK(uri_parts.path() == U"/hello");
CHECK(uri_parts.query() == U"query");
CHECK(uri_parts.fragment() == U"fragment");

CHECK(uri_parts.is_http());
CHECK(!uri_parts.is_authority());
CHECK(!uri_parts.is_absolute());
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
