## Include

```c++
#include <foxy/pct_decode.hpp>
```

## `foxy::pct_decode`

#### Declaration

```c++
template <class OutputIterator>
auto
pct_decode(boost::string_view const str, OutputIterator sink) -> OutputIterator;
```

#### Synopsis

Writes the pct-dedoded string denoted by `str` to the supplied OutputIterator.

Will stop writing upon first error (consecutive `'%'` characters).

This function does no validation over its inputs so the output string may contain invalid utf-8
bytes. This case is left up to the user to manage.

#### Example

```c++
// valid input
//
auto const input = boost::string_view("www.%c5%bc%c3%b3%c5%82%c4%87.pl");

auto bytes = std::array<char, 256>{0};

auto const decoded_view =
  boost::string_view(bytes.data(), foxy::uri::pct_decode(input, bytes.begin()) - bytes.begin());

CHECK(decoded_view == u8"www.\u017C\u00F3\u0142\u0107.pl");

// invalid input
//
auto const invalid_host = boost::string_view("www.goo%%%%gle.com");

auto bytes = std::array<char, 256>{0};

auto const decoded_view = boost::string_view(
  bytes.data(), foxy::uri::pct_decode(invalid_host, bytes.begin()) - bytes.begin());

CHECK(decoded_view == "www.goo");
```

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
