# foxy::uri::encode_fragment

## Include

```c++
#include <foxy/pct_encode.hpp>
```
## Declaration

```c++
template <class OutputIterator>
auto
encode_fragment(boost::u32string_view const fragment, OutputIterator out) -> OutputIterator;
```

## Synopsis

Writes the percent-encoded fragment component of a URI to the provided OutputIterator.

This function is the same as `foxy::uri::encode_query`.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
