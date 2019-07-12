## foxy::basic_uri_parts

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

#### Defaults

```c++
basic_uri_parts();
basic_uri_parts(basic_uri_parts const&) = default;
basic_uri_parts(basic_uri_parts&&)      = default;
```

Default constructor value-initializes all views to being empty `nullptr` views.

## Member Functions

#### Copy-Assignment Operator

```c++
auto
operator=(basic_uri_parts const&) -> basic_uri_parts& = default;
```

#### Move-Assignment Operator

```c++
auto
operator=(basic_uri_parts&&) noexcept -> basic_uri_parts& = default;
```

#### scheme

```c++
auto
scheme() const noexcept -> string_view;
```

Returns the `scheme` portion of the parsed URI. Empty if does not exist.

#### host

```c++
auto
host() const noexcept -> string_view;
```

Returns the `host` portion of the URI.

#### port

```c++
auto
port() const noexcept -> string_view;
```

Returns the `port` portion of the URI. Empty if does not exist.

#### path

```c++
auto
path() const noexcept -> string_view;
```

Returns the `path` portion of the URI. Empty if does not exist.

#### query

```c++
auto
query() const noexcept -> string_view;
```

Returns the `query` portion of the URI. Empty if does not exist.

#### fragment

```c++
auto
fragment() const noexcept -> string_view;
```

Returns the `fragment` portion of the URI. Empty if does not exist.

#### is_http

```c++
auto
is_http() const noexcept -> bool;
```

Returns whether or not the parsed `scheme` portion matches either `"http"` or `"https"`.

#### is_authority

```c++
auto
is_authority() const noexcept -> bool;
```

Returns whether or not the parsed URI is in its authority form.

#### is_absolute

```c++
auto
is_absolute() const noexcept -> bool;
```

Returns whether or not the parsed URI is considered absolute by the RFC.

## Non-Member Functions

#### parse_uri

```c++
auto
parse_uri(boost::basic_string_view<char, std::char_traits<char>> const uri)
  -> basic_uri_parts<char>;

auto
parse_uri(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri)
  -> basic_uri_parts<char32_t>;
```

Parse the string denoted by `uri`. If a valid URI has been supplied, this function will return a
non-empty `basic_uri_parts`. Otherwise, a default-constructed instance is returned.

Example:

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

#### parse_complete

```c++
auto
parse_complete(boost::basic_string_view<char, std::char_traits<char>> const uri,
               basic_uri_parts<char>&                                       parts) -> bool;

auto
parse_complete(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri,
               basic_uri_parts<char32_t>&                                           parts) -> bool;
```

Attempt to parse a complete URI into the out-param `parts`. If parsing fails, `parts` will be left
in a potentially partially completed state so it is recommended to reset the state by re-assignging
a default constructed `basic_uri_parts`.

This function returns a boolean that indicates whether or not the parse was successful.

`parse_uri` should be preferred over this function.

#### parse_authority

```c++
auto
parse_authority(boost::basic_string_view<char, std::char_traits<char>> const uri,
                basic_uri_parts<char>&                                       parts) -> bool;

auto
parse_authority(boost::basic_string_view<char32_t, std::char_traits<char32_t>> const uri,
                basic_uri_parts<char32_t>&                                           parts) -> bool;
```

Attempt to parse the authority form of a URI into the out-param `parts`. If parsing fails, `parts`
will be left in a potentially partially completed state so it is recommended to reset the state by
re-assigning a default constructed `basic_uri_parts`.

This function returns a boolean that indicates whether or not the parse was successful.

`parse_uri` should be preferred over this function.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)

