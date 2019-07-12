## Include

```c++
#include <foxy/pct_encode.hpp>
```

## `foxy::utf8_encode`

#### Declaration

```c++
template <class OutputIterator>
auto
utf8_encode(char32_t const code_point, OutputIterator sink) -> OutputIterator;
```

#### Synopsis

Writes the uft-8 encoding of the Unicode code point denoted by `code_point` to the provided `sink`.

`sink` _must_ model [LegacyOutputIterator](https://en.cppreference.com/w/cpp/named_req/OutputIterator)
and must also be `char`-assignable.

Returns an iterator pointing to one-past-the-end of the last written `char`.

#### Example

```c++
auto buffer = std::array<std::uint8_t, 4>{0xff, 0xff, 0xff, 0xff};

auto const end = ::foxy::utf8_encode(code_point, buffer.begin());
```

#### Declaration

```c++
template <class InputIterator, class OutputIterator>
auto
utf8_encode(InputIterator begin, InputIterator end, OutputIterator sink) -> OutputIterator;
```

#### Synopsis

Writes the utf-8 encoding from the InputIterator range denoted by `begin` and `end` to the provided
`sink`. Returns an iterator to one-past-the-end of the last `char` written.

#### Example

```c++
auto utf8_bytes = std::vector<char>();
utf8_bytes.resize(bytes_per_code_point * code_points.size());

foxy::utf8_encode(code_points.begin(), code_points.end(), utf8_bytes.begin());
```

## `foxy::uri::encode_host`

#### Declaration

```c++
namespace uri
{
template <class OutputIterator>
auto
encode_host(boost::u32string_view const host, OutputIterator out) -> OutputIterator;
}
```

#### Synopsis

Writes the percent-encoded host component of a URI to the provided OutputIterator.

#### Example

```c++
auto const host = boost::u32string_view(U"hello world!\n");
auto       out  = std::string(256, '\0');

auto const encoded_host =
  boost::string_view(out.data(), foxy::uri::encode_host(host, out.begin()) - out.begin());

CHECK(encoded_host == "hello%20world!%0a");
```

## `foxy::uri::encode_path`

#### Declaration

```c++
namespace uri
{
template <class OutputIterator>
auto
encode_path(boost::u32string_view const path, OutputIterator out) -> OutputIterator;
}
```

#### Synopsis

Writes the percent-encoded path component of a URI to the provided OutputIterator.

#### Example

```c++
auto const path = boost::u32string_view(U"/\"#<>?@[\\]^`{|}:::");
auto       out  = std::string(256, '\0');

auto const encoded_path =
  boost::string_view(out.data(), foxy::uri::encode_path(path, out.begin()) - out.begin());

CHECK(encoded_path == "/%22%23%3c%3e%3f@%5b%5c%5d%5e%60%7b%7c%7d:::");
```

## `foxy::uri::encode_query`

##### Declaration

```c++
template <class OutputIterator>
auto
encode_query(boost::u32string_view const query, OutputIterator out) -> OutputIterator;
```

#### Synopsis

Writes the percent-encoded query component of a URI to the provided OutputIterator.

#### Example

```c++
auto const query = boost::u32string_view(U"?//\":#/<>?@[\\]^`{|}");
auto       out   = std::string(256, '\0');

auto const encoded_query =
  boost::string_view(out.data(), foxy::uri::encode_query(query, out.begin()) - out.begin());

CHECK(encoded_query == "?//%22%3a%23/%3c%3e?%40%5b%5c%5d%5e%60%7b%7c%7d");
```

## `foxy::uri::encode_fragment`

#### Declaration

```c++
template <class OutputIterator>
auto
encode_fragment(boost::u32string_view const fragment, OutputIterator out) -> OutputIterator;
```

#### Synopsis

Writes the percent-encoded fragment component of a URI to the provided OutputIterator.

This function is the same as `foxy::uri::encode_query`.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
