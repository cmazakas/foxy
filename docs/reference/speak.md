# foxy::speak

## Include

```c++
#include <foxy/speak.hpp>
```

## Declaration

```c++
template <class RequestFactory, class Allocator = std::allocator<char>>
auto
speak(boost::asio::any_io_executor ex,
      std::string           host_,
      std::string           service_,
      RequestFactory&&      request_factory_,
      session_opts          opts_ = {},
      Allocator const       alloc = {});
```

## Synopsis

`foxy::speak` is an allocator-aware client function that will perform DNS resolution for `host_` and
`service_`, deferring to the user's `request_factory_` to generate client HTTP code.

`speak` will generate a `foxy::client_session` with the passed-in `opts_`, perform
`async_connect(host_, service_)` and then invoke the `request_factory_` with the newly-created
`client_session`, feeding the return to `asio::async_compose`. Once the call to
`asio::async_compose` finishes, the connection is gracefully shutdown.

If there's an error in the setup of the connection, the user's specified callback will be invoked
with a truthy error code.

`foxy::speak` manages the setup and teardown of client HTTP(S) connections. It enables the user
to solely focus on using Beast without any need to touch Asio directly.

## Example

```c++
SECTION("speak should function as a basic HTTP client")
{
  asio::io_context io{1};

  auto const* const host    = "www.google.com";
  auto const* const service = "http";

  auto executor = io.get_executor();

  foxy::speak(executor, host, service, [](auto& client_session) {
    auto req_ = std::make_unique<http::request<http::empty_body>>(http::verb::get, "/", 11);
    req_->set(http::field::host, "www.google.com");

    auto res_ = std::make_unique<http::response<http::string_body>>();

    return [&client_session, coro = asio::coroutine(), req = std::move(req_),
            res = std::move(res_)](auto& self, boost::system::error_code ec = {},
                                    std::size_t bytes_transferred = 0) mutable {
      reenter(coro)
      {
        yield client_session.async_request(*req, *res, std::move(self));
        CHECK(res->result() == http::status::ok);
        CHECK(res->body().size() > 0);

        return self.complete({}, 0);
      }
    };
  });

  io.run();
}
```

---

To [Reference](../reference.md#Reference)

To [ToC](../index.md#Table-of-Contents)
