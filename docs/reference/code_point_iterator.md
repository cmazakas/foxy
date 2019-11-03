# foxy::code_point_iterator

## Include

```c++
#include <foxy/code_point_iterator.hpp>
```

## Synopsis

The `code_point_iterator` is a simple InputIterator used to traverse a range of arbitrary character
type and output Unicode code points in the form of `char32_t`.

The `code_point_iterator` is an "iterator adapter" and wraps an iterator pair as it must know when
the range terminates.

See [operator*](#dereference) for details on behavior.

## Declaration

```c++
template <class Iterator>
struct code_point_iterator;
```

## Member Typedefs

```c++
using value_type        = char32_t;
using difference_type   = std::ptrdiff_t;
using pointer           = void;
using reference         = value_type;
using iterator_category = std::input_iterator_tag;
```

## Constructors

### Defaults

```c++
code_point_iterator(void)                           = default;
code_point_iterator(code_point_iterator const&)     = default;
code_point_iterator(code_point_iterator&&) noexcept = default;
```

### Iterator Pair

```c++
code_point_iterator(Iterator begin, Iterator end)
```

The main constructor for the iterator type. The iterator begins wherever `begin` is and uses the
`end` iterator as a terminating condition.

The behavior is undefined if `begin` is advanced and never compares equal to `end`.

## Member Functions

### Copy Assignment

```c++
auto
operator=(code_point_iterator const& rhs) & -> code_point_iterator&;
```

Copy-assignment operator

### Move Assignment

```c++
auto
operator=(code_point_iterator&& rhs) & noexcept -> code_point_iterator&;
```

Move-assignment operator

### Dereference

```c++
auto
operator*() -> value_type;
```

Dereference operator. Internally mutates the iterator, advancing it until either a valid code point
is found or if one cannot be found, an invalid code point will be returned.

Returns `0xFFFFFFFFu` when the character sequence contains an illegal code point and then returns
`0xFFFFFFFEu` for an incomplete code point, i.e. 2 out of 3 valid utf-8 bytes were found but then
the string abruptly ends.

The internal iterator will point to the last consumed character thus enabling the following:

```c++
SECTION("should handle invalid and valid data in one string")
{
  // 0xff is an invalid utf-8 sequence
  // 'a' and 'b', however, are valid
  //
  auto const input = std::array<char, 3>{'\xff', 'a', 'b'};

  auto       pos = foxy::code_point_iterator<decltype(input.begin())>(input.begin(), input.end());
  auto const end = foxy::code_point_iterator<decltype(input.begin())>(input.end(), input.end());

  CHECK(*pos++ == 0xFFFFFFFFu);
  CHECK(*pos++ == U'a');
  CHECK(*pos++ == U'b');

  CHECK(pos == end);
}
```

### Pre-Increment

```c++
auto
operator++() & -> code_point_iterator&;
```

No-op as traversal is done via `operator*`.

### Post-Increment

```c++
auto
operator++(int const) & -> code_point_iterator&;
```

No-op as traversal is done via `operator*`.

### Equality

```c++
auto
operator==(code_point_iterator const& rhs) const noexcept -> bool;
```

Returns whether the nested iterator is equal to the one in `rhs`.

### Inequality

```c++
auto
operator!=(code_point_iterator const& rhs) const noexcept -> bool
```

Returns the negation of `operator==`.

### Swap

```c++
auto
swap(code_point_iterator& rhs) -> void;
```

Swaps the internal iterator with the one in `rhs`.

## Non-Member Functions

### swap

```c++
namespace code_point
{
template <class Iterator>
auto
swap(code_point_iterator<Iterator>& iter1, code_point_iterator<Iterator>& iter2);
} // namespace code_point
```

Invokes `iter1.swap(iter2)`. Intended to work as a swap-friendly customization hook for `std::swap`
and the STL.

### make_code_point_iterator

```c++
template <class Iterator>
auto
make_code_point_iterator(Iterator const begin, Iterator const end)
  -> code_point_iterator<Iterator>;
```

Factory function used to more ergonomically create code point iterators due to function template
deduction.

Returns a code point iterator constructed as-if: `foxy::code_point_iterator(begin, end)`.

See also:
 * [code_point_view](./code_point_view.md#foxy::code_point_view)

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
