target_sources(
  foxy

  PRIVATE
    foxy.hpp

    foxy/client_session.hpp
    foxy/code_point_iterator.hpp
    foxy/code_point_view.hpp
    foxy/error.hpp
    foxy/listener.hpp
    foxy/log.hpp
    foxy/multi_stream.hpp
    foxy/parse_uri.hpp
    foxy/pct_decode.hpp
    foxy/pct_encode.hpp
    foxy/proxy.hpp
    foxy/server_session.hpp
    foxy/session_opts.hpp
    foxy/session.hpp
    foxy/speak.hpp
    foxy/type_traits.hpp
    foxy/uri_parts.hpp
    foxy/uri.hpp
    foxy/utf8.hpp
    foxy/utility.hpp

    foxy/detail/close_stream.hpp
    foxy/detail/export_connect_fields.hpp
    foxy/detail/has_token.hpp
    foxy/detail/relay.hpp
    foxy/detail/timed_op_wrapper_v3.hpp
    foxy/detail/tunnel.hpp

    foxy/impl/session.impl.hpp

    foxy/impl/client_session/async_connect.impl.hpp
    foxy/impl/client_session/async_request.impl.hpp

    foxy/impl/server_session/async_detect_ssl.impl.hpp
    foxy/impl/server_session/async_handshake.impl.hpp

    foxy/impl/session/async_read.impl.hpp
    foxy/impl/session/async_read_header.impl.hpp
    foxy/impl/session/async_write.impl.hpp
    foxy/impl/session/async_write_header.impl.hpp
)

target_include_directories(
  foxy

  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}>
)
