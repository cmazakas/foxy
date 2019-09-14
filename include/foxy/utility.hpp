//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#ifndef FOXY_UTILITY_HPP_
#define FOXY_UTILITY_HPP_

#include <boost/asio/ssl/context.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/ssl/error.hpp>

#include <boost/utility/string_view.hpp>

#include <boost/throw_exception.hpp>

#include <boost/system/error_code.hpp>

#include <openssl/x509_vfy.h>
#include <openssl/x509.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <string>

namespace foxy
{
// this namespace is a port of the up-and-coming library Boost.Certify:
// https://github.com/djarek/certify/tree/b5d8b2ed67fd12437e8b2f104477b2d06c3d6803
//
// written by Damian Jarek, distributed under the Boost Software License found here in the source:
// https://github.com/djarek/certify/blob/b5d8b2ed67fd12437e8b2f104477b2d06c3d6803/LICENSE_1_0.txt
//
// without their tireless research, Foxy wouldn't be nearly as secure or useful as it is
//
namespace certify
{
extern "C" int
verify_server_certificates(::X509_STORE_CTX* ctx, void*) noexcept;

auto
enable_https_verification(boost::asio::ssl::context& ssl_ctx) -> void;

auto
set_server_hostname(::SSL*                     ssl,
                    boost::string_view         hostname,
                    unsigned int               flags,
                    boost::system::error_code& ec) -> void;

auto
set_server_hostname(::SSL* ssl, boost::string_view hostname, unsigned int flags = 0) -> void;

template <class AsyncSSLStream>
void
set_sni_hostname(AsyncSSLStream& stream, std::string const& hostname, boost::system::error_code& ec)
{
  if (!SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str())) {
    ec = {static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
    return;
  }

  ec = {};
}

template <class AsyncSSLStream>
void
set_sni_hostname(AsyncSSLStream& stream, std::string const& hostname)
{
  auto ec = boost::system::error_code();

  if (!SSL_set_tlsext_host_name(stream.native_handle(), hostname.c_str())) {
    ec = {static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
  } else {
    ec = {};
  }

  if (ec) { boost::throw_exception(boost::system::system_error(ec)); }
}
} // namespace certify

template <class... Args>
auto
make_ssl_ctx(Args&&... args) -> boost::asio::ssl::context
{
  auto ctx = boost::asio::ssl::context(std::forward<Args>(args)...);

  foxy::certify::enable_https_verification(ctx);
  ctx.set_verify_mode(boost::asio::ssl::context::verify_peer |
                      boost::asio::ssl::context::verify_fail_if_no_peer_cert);

  return ctx;
}

} // namespace foxy

#endif // FOXY_UTILITY_HPP_
