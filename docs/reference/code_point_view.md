## `foxy::code_point_view`

## Include

```c++
#include <foxy/iterator.hpp>
```

## Synopsis

A range-like type that makes working with code point iterators easier.

## Declaration

```c++
template <class Char, class Traits = std::char_traits<Char>>
struct code_point_view;
```

## Member Typedefs

```c++
using iterator_type = typename boost::basic_string_view<Char, Traits>::iterator;
```

## Constructors

#### Defaults

```c++
code_point_view()                       = default;
code_point_view(code_point_view const&) = default;
code_point_view(code_point_view&&)      = default;
```

#### string_view

```c++
code_point_view(boost::basic_string_view<Char, Traits> view)
```

Constructs the code point view with the provided string view. This view is used to produce the range
of Unicode code points (`char32_t`).

## Member Functions

#### begin

```c++
auto
begin() const noexcept -> code_point_iterator<iterator_type>;
```

Returns a code point iterator that starts at the beginning of the internal string view.

#### end

```c++
auto
end() const noexcept -> code_point_iterator<iterator_type>;
```

Returns a code point iterator that represents the end of the possible code point range.

See also:
 * [code_point_iterator](./code_point_iterator.md#foxy::code_point_iterator)

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
