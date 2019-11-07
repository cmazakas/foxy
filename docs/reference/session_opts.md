# foxy::session_opts

## Include

```c++
#include <foxy/session_opts.hpp>
```

## Synopsis

A small class that Foxy uses for configuring its `basic_session` class and descendants.

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
// The session opts do not own the SSL context that is provided. It is the user's responsibility to
// ensure that the SSL context outlives the `session_opts` instance.
//
boost::optional<boost::asio::ssl::context&> ssl_ctx          = {};

// The `timeout` member serves as a relative deadline for an asynchronous operation, i.e. if a value
// of 30 seconds is supplied, `foxy::basic_session<Stream>::async_read` had 30 seconds to complete
// in its entirety or the pending operation will be invoked with an `operation_aborted` error code.
//
// The `timeout` can be safely adjusted any time in-between the `basic_session::async_*` methods.
// For example:
//
// auto client = foxy::client_session(...);
//
// client.opts.timeout = 10s;
//
// // 10 seconds to read in the response
// client.async_read(...);
//
// client.opts.timeout = 30s;
//
// // this time wait 30 seconds for writing the request
// client.async_write(...);
//
duration_type                               timeout          = std::chrono::seconds{1};

// *** Currently only affects the foxy::basic_client_session ***
//
// If the session options contain an SSL context and this parameter is set to true, the
// `foxy::basic_client_session` will attempt to verify the certificate sent by the remote
//
// If set to false and the SSL context is not null, no certificate verification will be performed
// but the TLS handshake will still be performed
//
// This is considered insecure and should not be used in production without good reason
//
bool                                        verify_peer_cert = true;
```

## Constructors

None provided (class is an aggregate).

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
