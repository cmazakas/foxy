# foxy::utf8_encode

## Include

```c++
#include <foxy/utf8.hpp>
```

## Declaration

```c++
template <class OutputIterator>
auto
utf8_encode(char32_t const code_point, OutputIterator sink) -> OutputIterator;
```

## Synopsis

Writes the uft-8 encoding of the Unicode code point denoted by `code_point` to the provided `sink`.

`sink` _must_ model [LegacyOutputIterator](https://en.cppreference.com/w/cpp/named_req/OutputIterator)
and must also be `char`-assignable.

Returns an iterator pointing to one-past-the-end of the last written `char`.

## Example

```c++
auto buffer = std::array<std::uint8_t, 4>{0xff, 0xff, 0xff, 0xff};

auto const end = ::foxy::utf8_encode(code_point, buffer.begin());
```

---

## Declaration

```c++
template <class InputIterator, class OutputIterator>
auto
utf8_encode(InputIterator begin, InputIterator end, OutputIterator sink) -> OutputIterator;
```

## Synopsis

Writes the utf-8 encoding from the InputIterator range denoted by `begin` and `end` to the provided
`sink`. Returns an iterator to one-past-the-end of the last `char` written.

## Example

```c++
auto utf8_bytes = std::vector<char>();
utf8_bytes.resize(bytes_per_code_point * code_points.size());

foxy::utf8_encode(code_points.begin(), code_points.end(), utf8_bytes.begin());
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
