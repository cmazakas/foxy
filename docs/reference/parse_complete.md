# foxy::parse_complete

## Include

```c++
#include <foxy/parse_uri.hpp>
```

## Declaration

```c++
auto
parse_complete(boost::basic_string_view<char, std::char_traits<char>> const uri,
               basic_uri_parts<char>&                                       parts) -> bool;

auto
parse_complete(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri,
               basic_uri_parts<char32_t>&                                           parts) -> bool;
```

## Synopsis

Attempt to parse a complete URI into the out-param `parts`. If parsing fails, `parts` will be left
in a potentially partially completed state so it is recommended to reset the state by re-assignging
a default constructed `basic_uri_parts`.

This function returns a boolean that indicates whether or not the parse was successful.

`parse_uri` should be preferred over this function.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
