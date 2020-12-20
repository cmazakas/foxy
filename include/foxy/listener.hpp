//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/server_session.hpp>
#include <foxy/log.hpp>
#include <foxy/code_point_view.hpp>

#include <boost/asio/executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/post.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/context.hpp>

#include <boost/beast/http/error.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/core/bind_handler.hpp>

#include <boost/optional/optional.hpp>

#include <memory>
#include <type_traits>

#include <boost/assert.hpp>

namespace foxy
{
namespace detail
{
template <class RequestHandler>
struct server_op : boost::asio::coroutine
{
  using executor_type = boost::asio::strand<boost::asio::any_io_executor>;

  struct frame
  {
    std::unique_ptr<::foxy::server_session>                            server_handle;
    RequestHandler                                                     handler;
    boost::beast::http::request_parser<boost::beast::http::empty_body> shutdown_parser;

    frame(std::unique_ptr<::foxy::server_session>&& server_handle_, RequestHandler&& handler_)
      : server_handle(std::move(server_handle_))
      , handler(std::move(handler_))
    {
    }
  };

  std::unique_ptr<frame> frame_ptr;
  executor_type          strand;

  server_op(std::unique_ptr<::foxy::server_session>&& server_handle_, RequestHandler&& handler_)
    : frame_ptr(std::make_unique<frame>(std::move(server_handle_), std::move(handler_)))
    , strand(boost::asio::make_strand(frame_ptr->server_handle->get_executor()))
  {
  }

  auto operator()(boost::system::error_code ec = {}, std::size_t const bytes_transferred = 0)
    -> void
  {
    BOOST_ASSERT(strand.running_in_this_thread());

    auto& f      = *frame_ptr;
    auto& server = *f.server_handle;
    BOOST_ASIO_CORO_REENTER(*this)
    {
      if (server.stream.is_ssl()) {
        BOOST_ASIO_CORO_YIELD
        server.async_handshake(std::move(*this));
        if (ec) { goto shutdown; }
      }

      BOOST_ASIO_CORO_YIELD
      boost::asio::async_compose<std::decay_t<decltype(*this)>,
                                 void(boost::system::error_code, std::size_t)>(std::move(f.handler),
                                                                               *this, strand);

      if (ec) { goto shutdown; }

    shutdown:
      BOOST_ASIO_CORO_YIELD server.async_shutdown(std::move(*this));
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
}; // namespace detail

template <class RequestHandlerFactory>
struct accept_op : boost::asio::coroutine
{
  using executor_type = boost::asio::strand<boost::asio::any_io_executor>;

  struct frame
  {
    boost::asio::ip::tcp::socket socket;
    RequestHandlerFactory        factory;

    frame(boost::asio::any_io_executor executor, RequestHandlerFactory&& factory_)
      : socket(executor)
      , factory(std::move(factory_))
    {
    }
  };

  boost::asio::ip::tcp::acceptor&             acceptor;
  std::unique_ptr<frame>                      frame_ptr;
  executor_type                               strand;
  boost::optional<boost::asio::ssl::context&> ctx;

  accept_op(boost::asio::ip::tcp::acceptor& acceptor_,
            executor_type                   strand_,
            RequestHandlerFactory&&         factory_)
    : acceptor(acceptor_)
    , frame_ptr(std::make_unique<frame>(acceptor.get_executor(), std::move(factory_)))
    , strand(strand_)
  {
  }

  accept_op(boost::asio::ip::tcp::acceptor& acceptor_,
            executor_type                   strand_,
            boost::asio::ssl::context&      ctx_,
            RequestHandlerFactory&&         factory_)
    : acceptor(acceptor_)
    , frame_ptr(std::make_unique<frame>(acceptor.get_executor(), std::move(factory_)))
    , strand(strand_)
    , ctx(ctx_)
  {
  }

  auto operator()(boost::system::error_code ec = {}) -> void
  {
    BOOST_ASSERT(strand.running_in_this_thread());

    auto& f = *frame_ptr;
    BOOST_ASIO_CORO_REENTER(*this)
    {
      while (acceptor.is_open()) {
        BOOST_ASIO_CORO_YIELD acceptor.async_accept(f.socket, std::move(*this));
        if (ec) {
          if (ec != boost::asio::error::operation_aborted) {
            ::foxy::log_error(ec, "foxy::listener::accept_op");
          }

          return;
        }

        {
          auto session_handle = std::make_unique<::foxy::server_session>(
            ctx ? ::foxy::multi_stream(std::move(f.socket), *ctx)
                : ::foxy::multi_stream(std::move(f.socket)),
            ::foxy::session_opts{ctx, std::chrono::seconds{30}, false});

          auto handler = f.factory(*session_handle);

          using handler_type = std::decay_t<decltype(handler)>;

          boost::asio::post(server_op<handler_type>(std::move(session_handle), std::move(handler)));
        }
      }
    }
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand;
  }
};
} // namespace detail

struct listener
{
public:
  using executor_type = boost::asio::strand<boost::asio::any_io_executor>;

private:
  boost::asio::ip::tcp::acceptor             acceptor_;
  executor_type                              strand_;
  boost::optional<boost::asio::ssl::context> ctx_;

public:
  listener()                = delete;
  listener(listener const&) = delete;
  listener(listener&&)      = default;

  listener(boost::asio::any_io_executor executor, boost::asio::ip::tcp::endpoint endpoint)
    : acceptor_(executor, endpoint)
    , strand_(boost::asio::make_strand(executor))
  {
  }

  listener(boost::asio::any_io_executor   executor,
           boost::asio::ip::tcp::endpoint endpoint,
           boost::asio::ssl::context      ctx)
    : acceptor_(executor, endpoint)
    , strand_(boost::asio::make_strand(executor))
    , ctx_(std::move(ctx))
  {
  }

  auto
  get_executor() const noexcept -> executor_type
  {
    return strand_;
  }

  template <class RequestHandlerFactory>
  auto
  async_accept(RequestHandlerFactory&& factory) -> void
  {
    if (ctx_) {
      return boost::asio::post(
        detail::accept_op<RequestHandlerFactory>(acceptor_, strand_, *ctx_, std::move(factory)));
    }

    boost::asio::post(
      detail::accept_op<RequestHandlerFactory>(acceptor_, strand_, std::move(factory)));
  }

  auto
  shutdown() -> void
  {
    boost::asio::post(get_executor(), [self = this]() mutable -> void {
      auto ec = boost::system::error_code();

      self->acceptor_.cancel(ec);
      self->acceptor_.close(ec);
    });
  }
};
} // namespace foxy
