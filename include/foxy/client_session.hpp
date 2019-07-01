//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_CLIENT_SESSION_HPP_
#define FOXY_CLIENT_SESSION_HPP_

#include <foxy/session.hpp>
#include <foxy/type_traits.hpp>
#include <foxy/shared_handler_ptr.hpp>
#include <foxy/detail/timed_op_wrapper_v2.hpp>

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
#include <boost/beast/core/bind_handler.hpp>

#include <boost/optional/optional.hpp>
#include <boost/smart_ptr/make_shared.hpp>

#include <boost/container/container_fwd.hpp>

#include <chrono>
#include <string>
#include <utility>
#include <memory>
#include <functional>

namespace foxy
{
template <class DynamicBuffer>
struct basic_client_session
  : public basic_session<
      boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                       typename boost::asio::io_context::executor_type>,
      DynamicBuffer>
{
public:
  basic_client_session()                            = delete;
  basic_client_session(basic_client_session const&) = delete;
  basic_client_session(basic_client_session&&)      = default;

  template <class... BufferArgs>
  basic_client_session(boost::asio::io_context& io, session_opts opts, BufferArgs&&... bargs)
    : basic_session<
        boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                         typename boost::asio::io_context::executor_type>,
        DynamicBuffer>(io, std::move(opts), std::forward<BufferArgs>(bargs)...)
  {
  }

  template <class ConnectHandler>
  auto
  async_connect(std::string host, std::string service, ConnectHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<ConnectHandler>,
                                       void(boost::system::error_code,
                                            boost::asio::ip::tcp::endpoint)>::return_type;

  template <class Request, class ResponseParser, class RequestHandler>
  auto
  async_request(Request& request, ResponseParser& parser, RequestHandler&& handler) & ->
    typename boost::asio::async_result<std::decay_t<RequestHandler>,
                                       void(boost::system::error_code)>::return_type;
};

using client_session = basic_client_session<boost::beast::flat_buffer>;

namespace pmr
{
template <class T>
using client_session = basic_client_session<
  boost::beast::basic_flat_buffer<boost::container::pmr::polymorphic_allocator<T>>>;
}

} // namespace foxy

#include <foxy/impl/client_session/async_connect.impl.hpp>
#include <foxy/impl/client_session/async_request.impl.hpp>

#endif // FOXY_CLIENT_SESSION_HPP_
