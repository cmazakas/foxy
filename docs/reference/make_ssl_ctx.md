# `foxy::make_ssl_ctx`

## Include

```c++
#include <foxy/utility.hpp>
```

## Declaration

```c++
template <class... Args>
auto
make_ssl_ctx(Args&&... args) -> boost::asio::ssl::context;
```

## Synopsis

A client-oriented factory function that returns an `asio::ssl::context` by first forwarding `args`
to it and then requiring peer certificate verification.

Suitable for usage with `foxy::basic_client_session` or `foxy::proxy`.

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
