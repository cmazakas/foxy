#ifndef FOXY_CLIENT_SESSION_HPP_
#define FOXY_CLIENT_SESSION_HPP_

#include "foxy/session.hpp"
#include "foxy/shared_handler_ptr.hpp"

#include <boost/system/error_code.hpp>

#include <boost/asio/async_result.hpp>
#include <boost/asio/associated_executor.hpp>
#include <boost/asio/associated_allocator.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <boost/asio/post.hpp>
#include <boost/asio/coroutine.hpp>

#include <boost/asio/connect.hpp>

#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/core/handler_ptr.hpp>
#include <boost/beast/core/bind_handler.hpp>

#include <boost/optional/optional.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <iostream>
#include <chrono>
#include <string>
#include <utility>
#include <memory>
#include <functional>

namespace foxy
{

struct client_session : public session
{
public:
  client_session()                      = delete;
  client_session(client_session const&) = default;
  client_session(client_session&&)      = default;

  explicit client_session(boost::asio::io_context& io);

  template <class ConnectHandler>
  auto
  async_connect(
    std::string      host,
    std::string      service,
    ConnectHandler&& handler
  ) & -> BOOST_ASIO_INITFN_RESULT_TYPE(
    ConnectHandler, void(boost::system::error_code, boost::asio::ip::tcp::endpoint));

  template <class Request, class ResponseParser, class RequestHandler>
  auto
  async_request(
    Request&         request,
    ResponseParser&  parser,
    RequestHandler&& handler
  ) & -> BOOST_ASIO_INITFN_RESULT_TYPE(
    RequestHandler, void(boost::system::error_code));
};

} // foxy

#include "foxy/impl/client_session.impl.hpp"

#endif // FOXY_CLIENT_SESSION_HPP_
