#ifndef FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
#define FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_

#include "foxy/client_session.hpp"

namespace foxy
{
namespace detail
{

template <class ConnectHandler>
struct connect_op : boost::asio::coroutine
{
private:

  struct state
  {
    ::foxy::client_session&                      session;
    std::string                                  host;
    std::string                                  service;
    boost::asio::ip::tcp::resolver               resolver;
    boost::asio::ip::tcp::resolver::results_type results;
    boost::asio::ip::tcp::endpoint               endpoint;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      ConnectHandler const&   handler,
      ::foxy::client_session& session_,
      std::string             host_,
      std::string             service_)
    : session(session_)
    , host(std::move(host_))
    , service(std::move(service_))
    , resolver(session.stream.get_executor().context())
    , work(session.get_executor())
    {
    }
  };

  boost::beast::handler_ptr<state, ConnectHandler> p_;

public:
  connect_op()                  = delete;
  connect_op(connect_op const&) = default;
  connect_op(connect_op&&)      = default;

  template <class DeducedHandler>
  connect_op(
    ::foxy::client_session& session,
    std::string             host,
    std::string             service,
    DeducedHandler&&        handler)
  : p_(
    std::forward<DeducedHandler>(handler),
    session, std::move(host), std::move(service))
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    ConnectHandler,
    decltype(std::declval<::foxy::client_session&>().get_executor())
  >;

  using allocator_type = boost::asio::associated_allocator_t<ConnectHandler>;

  auto get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
  }

  auto get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  struct on_resolve_t {};
  struct on_connect_t {};

  auto operator()(
    on_resolve_t,
    boost::system::error_code                    ec,
    boost::asio::ip::tcp::resolver::results_type results) -> void;

  auto operator()(
    on_connect_t,
    boost::system::error_code      ec,
    boost::asio::ip::tcp::endpoint endpoint) -> void;

  auto operator()(
    boost::system::error_code ec,
    std::size_t const         bytes_transferred,
    bool const                is_continuation = true) -> void;
};

template <class ConnectHandler>
auto
connect_op<ConnectHandler>::
operator()(
  on_resolve_t,
  boost::system::error_code                    ec,
  boost::asio::ip::tcp::resolver::results_type results) -> void
{
  p_->results = std::move(results);
  (*this)(ec, 0);
}

template <class ConnectHandler>
auto
connect_op<ConnectHandler>::
operator()(
  on_connect_t,
  boost::system::error_code      ec,
  boost::asio::ip::tcp::endpoint endpoint) -> void
{
  p_->endpoint = std::move(endpoint);
  (*this)(ec, 0);
}

template <class ConnectHandler>
auto
connect_op<ConnectHandler>::
operator()(
  boost::system::error_code ec,
  std::size_t const         bytes_transferred,
  bool const                is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(*this)
  {
    if (s.session.stream.is_ssl()) {
      if (!SSL_set_tlsext_host_name(s.session.stream.ssl().native_handle(), s.host.c_str())) {
          ec.assign(static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
          goto upcall;
      }
    }

    BOOST_ASIO_CORO_YIELD
    s.resolver.async_resolve(
      s.host, s.service, bind_handler(std::move(*this), on_resolve_t{}, _1, _2));
    if (ec) { goto upcall; }

    BOOST_ASIO_CORO_YIELD
    boost::asio::async_connect(
      s.session.stream.plain(), s.results,
      bind_handler(std::move(*this), on_connect_t{}, _1, _2));

    if (ec) { goto upcall; }

    {
      auto endpoint = std::move(s.endpoint);
      auto work     = std::move(s.work);
      return p_.invoke(boost::system::error_code(), std::move(endpoint));
    }

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(boost::beast::bind_handler(std::move(*this), ec, 0));
    }
    auto work = std::move(s.work);
    p_.invoke(ec, boost::asio::ip::tcp::endpoint());
  }
}


template <class ConnectHandler>
struct connect_op_main
{
private:

  // TODO: switch to `std::basic_string<char, get_allocator()>`
  //
  struct state
  {
    ::foxy::client_session&        session;

    // these can likely be refactored into a:
    // variant<pair<string, strong>, tcp::endpoint>
    //
    std::string                    host;
    std::string                    service;
    boost::asio::ip::tcp::endpoint endpoint;

    int                            ops_completed;
    boost::asio::coroutine         coro;

    boost::asio::executor_work_guard<decltype(session.get_executor())> work;

    explicit state(
      ConnectHandler const&   handler_,
      ::foxy::client_session& session_,
      std::string             host_,
      std::string             service_)
    : session(session_)
    , host(std::move(host_))
    , service(std::move(service_))
    , ops_completed{0}
    , coro()
    , work(session.get_executor())
    {
    }
  };

  ::foxy::shared_handler_ptr<state, ConnectHandler> p_;

public:
  connect_op_main()                       = delete;
  connect_op_main(connect_op_main const&) = default;
  connect_op_main(connect_op_main&&)      = default;

  template <class DeducedHandler>
  connect_op_main(
    ::foxy::client_session& session,
    std::string             host,
    std::string             service,
    DeducedHandler&&        handler)
  : p_(std::forward<DeducedHandler>(handler), session, std::move(host), std::move(service))
  {
  }

  using executor_type = boost::asio::associated_executor_t<
    ConnectHandler,
    decltype(std::declval<::foxy::client_session&>().get_executor())
  >;

  auto get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.get_executor());
  }

  using allocator_type = boost::asio::associated_allocator_t<ConnectHandler>;

  auto get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  struct on_connect_t {};
  struct on_timer_t {};

  auto operator()(
    on_connect_t,
    boost::system::error_code ec, boost::asio::ip::tcp::endpoint endpoint
  ) -> void;

  auto operator()(on_timer_t, boost::system::error_code ec) -> void;
  auto operator()(boost::system::error_code ec, bool is_continuation = true) -> void;
};

template <class ConnectHandler>
auto
connect_op_main<ConnectHandler>::
operator()(
  on_connect_t,
  boost::system::error_code      ec,
  boost::asio::ip::tcp::endpoint endpoint) -> void
{
  p_->ops_completed++;
  p_->endpoint = std::move(endpoint);

  auto ec2 = ec;
  p_->session.timer.cancel(ec2);

  (*this)(ec);
}

template <class ConnectHandler>
auto
connect_op_main<ConnectHandler>::
operator()(
  on_timer_t,
  boost::system::error_code ec) -> void
{
  p_->ops_completed++;
  if (ec == boost::asio::error::operation_aborted) {
    return (*this)(boost::system::error_code());
  }

  p_->session.stream.plain().close(ec);
  (*this)(ec);
}

template <class ConnectHandler>
auto
connect_op_main<ConnectHandler>::
operator()(
  boost::system::error_code ec,
  bool                      is_continuation) -> void
{
  using namespace std::placeholders;
  using boost::beast::bind_handler;

  auto& s = *p_;
  BOOST_ASIO_CORO_REENTER(s.coro)
  {
    s.session.timer.expires_after(s.session.opts.timeout);

    {
      auto h = bind_handler(*this, on_connect_t{}, _1, _2);
      connect_op<decltype(h)>(
        s.session, std::move(s.host), std::move(s.service), std::move(h)
      )({}, 0, false);
    }

    s.session.timer.async_wait(bind_handler(*this, on_timer_t{}, _1));

    p_.reset();
    while (s.ops_completed < 2) {
      BOOST_ASIO_CORO_YIELD;
    }

    if (ec) { goto upcall; }

    BOOST_ASIO_CORO_YIELD break;

  upcall:
    if (!is_continuation) {
      BOOST_ASIO_CORO_YIELD
      boost::asio::post(boost::beast::bind_handler(*this, ec));
    }
    BOOST_ASIO_CORO_YIELD break;
  }

  if (!s.coro.is_complete()) { return; }

  auto endpoint = std::move(s.endpoint);
  auto work     = std::move(s.work);

  if (ec) { return p_.invoke(ec, boost::asio::ip::tcp::endpoint()); };
  return p_.invoke(ec, endpoint);
}

} // detail

template <class ConnectHandler>
auto
client_session::async_connect(
  std::string      host,
  std::string      service,
  ConnectHandler&& handler
) & -> BOOST_ASIO_INITFN_RESULT_TYPE(
  ConnectHandler, void(boost::system::error_code, boost::asio::ip::tcp::endpoint))
{
  boost::asio::async_completion<
    ConnectHandler, void(boost::system::error_code, boost::asio::ip::tcp::endpoint)
  >
  init(handler);

  detail::connect_op_main<
    BOOST_ASIO_HANDLER_TYPE(
      ConnectHandler, void(boost::system::error_code, boost::asio::ip::tcp::endpoint))
  >(
    *this, std::move(host), std::move(service), std::move(init.completion_handler)
  )({}, false);

  return init.result.get();
}

} // foxy

#endif // FOXY_IMPL_CLIENT_SESSION_ASYNC_CONNECT_IMPL_HPP_
