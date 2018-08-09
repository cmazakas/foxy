#ifndef FOXY_CLIENT_SESSION_IMPL_HPP_
#define FOXY_CLIENT_SESSION_IMPL_HPP_

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

    explicit state(
      ConnectHandler const&   handler,
      ::foxy::client_session& session_,
      std::string             host_,
      std::string             service_)
    : session(session_)
    , host(std::move(host_))
    , service(std::move(service_))
    , resolver(session.tcp().get_executor().context())
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
    decltype(((std::declval<::foxy::client_session&>()).tcp().get_executor()))
  >;

  auto get_executor() const noexcept -> executor_type
  {
    return boost::asio::get_associated_executor(p_.handler(), p_->session.tcp().get_executor());
  }

  using allocator_type = boost::asio::associated_allocator_t<ConnectHandler>;

  auto get_allocator() const noexcept -> allocator_type
  {
    return boost::asio::get_associated_allocator(p_.handler());
  }

  struct on_resolve_t {};

  auto operator()(
    on_resolve_t,
    boost::system::error_code                    ec,
    boost::asio::ip::tcp::resolver::results_type results) -> void
  {
    p_->results = std::move(results);
    (*this)(ec, 0);
  }

  auto operator()(
    boost::system::error_code      ec,
    boost::asio::ip::tcp::endpoint endpoint) -> void
  {
    p_->endpoint = std::move(endpoint);
    (*this)(ec, 0);
  }

  #include <boost/asio/yield.hpp>
  auto operator()(
    boost::system::error_code ec,
    std::size_t const         bytes_transferred,
    bool const                is_continuation = true) -> void
  {
    auto& s = *p_;

    reenter(*this)
    {
      if (s.session.is_ssl()) {
        if (!SSL_set_tlsext_host_name(s.session.ssl().native_handle(), s.host.c_str())) {
            ec.assign(
              static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category());
            goto upcall;
        }
      }

      yield s.resolver.async_resolve(
        s.host, s.service, boost::beast::bind_handler(std::move(*this), on_resolve_t{}, std::placeholders::_1, std::placeholders::_2));
      if (ec) { goto upcall; }

      yield boost::asio::async_connect(s.session.tcp(), s.results, std::move(*this));
      if (ec) { goto upcall; }

      {
        auto endpoint = std::move(s.endpoint);
        return p_.invoke(boost::system::error_code(), std::move(endpoint));
      }

    upcall:
      if (!is_continuation) {
        yield boost::asio::post(boost::beast::bind_handler(std::move(*this), ec, 0));
      }
      p_.invoke(ec, boost::asio::ip::tcp::endpoint());
    }
  }
  #include <boost/asio/unyield.hpp>
};

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

  detail::connect_op<
    BOOST_ASIO_HANDLER_TYPE(
      ConnectHandler, void(boost::system::error_code, boost::asio::ip::tcp::endpoint))
  >(*this, std::move(host), std::move(service), init.completion_handler)({}, 0, false);

  return init.result.get();
}

} // foxy

#endif // FOXY_CLIENT_SESSION_IMPL_HPP_