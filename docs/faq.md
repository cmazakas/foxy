# Frequently Asked Questions

#### Does Foxy throw an exception when an HTTP response is >= 400?

Nope! When a `foxy::basic_session` finishes reading a response from the underlying stream, it only
assembles the message. This is the same that would happen if you called `boost::beast::async_read`
yourself.

Validation of the response status code is left as an exercise for the user.

Foxy will, however, invoke the `async_read` callback with a truth error code if there is an
underlying error in reading from the stream or parsing the response itself (i.e. content-length
headers don't match actual body length).

#### How Do I Set Timeouts?

Given:

```c++
auto session = foxy::client_session(io, {});
```

The timeout can be set like:

```c++
// 30 seconds to read in a header
//
session.opts.timeout = 30s;
session.async_read_header(req, yield);

// other code here...

// only 10 seconds to finish reading in the rest of the message
//
session.opts.timeout = 10s;
session.async_read(req, yield);
```

In this example, we give our session 30 seconds to finish parsing an entire header from the
session's underlying stream object. If the header could not be completely read within the time
limit, the timer operation will call `.close()` on the underlying socket.

In Foxy, every session method that begins with `async_` is timed. The time limit for each one of
these operations can be uniquely set by assigning the timeout before invoking them.

---

To [ToC](./index.md#Table-of-Contents)
