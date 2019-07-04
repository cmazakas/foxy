//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_SERVER_SESSION_HPP_
#define FOXY_SERVER_SESSION_HPP_

#include <foxy/session.hpp>

namespace foxy
{
template <class DynamicBuffer>
struct basic_server_session
  : public basic_session<
      boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                       typename boost::asio::io_context::executor_type>,
      DynamicBuffer>
{
public:
  basic_server_session()                            = delete;
  basic_server_session(basic_server_session const&) = delete;
  basic_server_session(basic_server_session&&)      = default;

  template <class... BufferArgs>
  basic_server_session(multi_stream stream_, session_opts opts, BufferArgs&&... bargs)
    : basic_session<
        boost::asio::basic_stream_socket<boost::asio::ip::tcp,
                                         typename boost::asio::io_context::executor_type>,
        DynamicBuffer>(std::move(stream_), opts, std::forward<BufferArgs>(bargs)...)
  {
  }
};

using server_session = basic_server_session<boost::beast::flat_buffer>;

} // namespace foxy

#endif // FOXY_SERVER_SESSION_HPP_
