## `foxy::session_opts`

## Synopsis

A small class that Foxy uses for configuring its `basic_session` class.

The session opts do not own the SSL context that is provided. It is the user's responsibility to
ensure that the SSL context has appropriate lifetime.

The `timeout` member serves as an absolute deadline for an asynchronous operation, i.e. if a value
of 30 seconds is supplied, `foxy::basic_session<Stream>::async_read` had 30 seconds to complete
in its entirety or the pending operation will be invoked with an `operation_aborted` error code.

The `timeout` can be adjusted at any time.

## Declaration

```c++
struct session_opts;
```

## Exported Typedefs

```c++
using duration_type = typename boost::asio::steady_timer::duration;
```

## Public Members

```c++
boost::optional<boost::asio::ssl::context&> ssl_ctx = {};
duration_type                               timeout = std::chrono::seconds{1};
```

## Constructors

None provided.

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
