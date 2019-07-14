# foxy::uri::pct_decode

## Include

```c++
#include <foxy/pct_decode.hpp>
```

## Declaration

```c++
namespace uri
{
template <class OutputIterator>
auto
pct_decode(boost::string_view const   str,
           OutputIterator             sink,
           boost::system::error_code& ec) noexcept -> OutputIterator;

// throwing version
//
template <class OutputIterator>
auto
pct_decode(boost::string_view const str, OutputIterator& sink) -> void;
}
```

## Synopsis

Writes the pct-dedoded string denoted by `str` to the supplied OutputIterator.

Will stop writing upon first error (consecutive `'%'` characters).

This function does no validation over its inputs so the output string may contain invalid utf-8
bytes. This case is left up to the user to manage.

The exceptional version will mutate its `sink` in-place and will throw an instance of
`foxy::error::unexpected_pct`.

The noexcept version can take its `sink` by-value and will instead write the error the user-supplied
`boost::system::error_code&` as is typical in Asio/Beast.

This design decision was to accomodate both workflows while also giving users the ability to probe
their buffers for partial decodings.

### Example

```c++
auto ec = boost::system::error_code{};

// clearly not a valid host
//
auto const invalid_host = boost::string_view("www.goo%%%%gle.com");

auto pos = invalid_host.begin();
x3::parse(pos, invalid_host.end(), foxy::uri::host);

// prove up-front that we do not have a full-match as X3 will mutate `pos` until it's equal to
// the end of the string view
//
auto const was_full_match = (pos == invalid_host.end());
REQUIRE(!was_full_match);

// output buffer to hold our utf-8 bytes
//
auto bytes = std::array<char, 256>{0};

// first decoded view using the noexcept API
//
auto decoded_view = boost::string_view(
  bytes.data(), foxy::uri::pct_decode(invalid_host, bytes.begin(), ec) - bytes.begin());

// "dirty" buffer (i.e. writes persist even in the face of errors)
//
CHECK(decoded_view == "www.goo");
CHECK(ec == foxy::error::unexpected_pct);

// reset the buffer
//
bytes.fill(0);

// now use the exceptional API
//
auto out = bytes.begin();
CHECK_THROWS(foxy::uri::pct_decode(invalid_host, out));

// note that even in the face of exceptions, we can still determine how much we actually wound up
// writing to our OutputIterator
//
decoded_view = boost::string_view(bytes.data(), out - bytes.begin());
CHECK(decoded_view == "www.goo");
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
