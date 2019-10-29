#include <foxy/session.hpp>
#include <foxy/detail/timed_op_wrapper_v3.hpp>

#include <boost/asio/io_context.hpp>

#include <catch2/catch.hpp>

namespace asio = boost::asio;

TEST_CASE("timed_op_wrapper_v3")
{
  asio::io_context io{1};

  auto session = foxy::session(io, foxy::session_opts{});

  auto impl = [&session](auto& self, boost::system::error_code ec = {},
                         std::size_t bytes_transferred = 0) mutable { self.complete({}, 0); };

  foxy::detail::async_timer<void(boost::system::error_code ec, std::size_t bytes_transferred)>(
    std::move(impl), session, [](boost::system::error_code ec, std::size_t bytes_transferred) {});

  io.run();
}
