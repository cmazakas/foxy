# foxy::basic_uri_parts

## Include

```c++
#include <foxy/uri_parts.hpp>
```

## Synopsis

`basic_uri_parts` is a collection of string views that enable the user to have a discretized view
over a URI.

The URI parts can only represent either a valid URI or an empty set of views. For a URI to be
considered valid it must either be in its complete form or its authority form.

This class is not meant to be directly constructed by users but instead returned from a set of
friend functions that will parse the URI string for the user.

## Declaration

```c++
template <class CharT>
struct basic_uri_parts;
```

The library only supports `char` and `char32_t` as template parameters.

## Member Typedefs

```c++
using string_view = boost::basic_string_view<CharT, std::char_traits<CharT>>;
using iterator    = typename string_view::iterator;
```

## Constructors

### Defaults

```c++
basic_uri_parts();
basic_uri_parts(basic_uri_parts const&) = default;
basic_uri_parts(basic_uri_parts&&)      = default;
```

Default constructor value-initializes all views to being empty `nullptr` views.

## Member Functions

### Copy-Assignment Operator

```c++
auto
operator=(basic_uri_parts const&) -> basic_uri_parts& = default;
```

### Move-Assignment Operator

```c++
auto
operator=(basic_uri_parts&&) noexcept -> basic_uri_parts& = default;
```

### scheme

```c++
auto
scheme() const noexcept -> string_view;
```

Returns the `scheme` portion of the parsed URI. Empty if does not exist.

### host

```c++
auto
host() const noexcept -> string_view;
```

Returns the `host` portion of the URI.

### port

```c++
auto
port() const noexcept -> string_view;
```

Returns the `port` portion of the URI. Empty if does not exist.

### path

```c++
auto
path() const noexcept -> string_view;
```

Returns the `path` portion of the URI. Empty if does not exist.

### query

```c++
auto
query() const noexcept -> string_view;
```

Returns the `query` portion of the URI. Empty if does not exist.

### fragment

```c++
auto
fragment() const noexcept -> string_view;
```

Returns the `fragment` portion of the URI. Empty if does not exist.

### is_http

```c++
auto
is_http() const noexcept -> bool;
```

Returns whether or not the parsed `scheme` portion matches either `"http"` or `"https"`.

### is_authority

```c++
auto
is_authority() const noexcept -> bool;
```

Returns whether or not the parsed URI is in its authority form.

### is_absolute

```c++
auto
is_absolute() const noexcept -> bool;
```

Returns whether or not the parsed URI is considered absolute by the RFC.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)

