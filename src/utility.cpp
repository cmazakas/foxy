//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/utility.hpp>

namespace asio = boost::asio;

extern "C" int
foxy::certify::verify_server_certificates(::X509_STORE_CTX* ctx, void*) noexcept
{
  auto const res = ::X509_verify_cert(ctx);
  if (res < 0) { return 0; }
  return res;
}

auto
foxy::certify::enable_https_verification(boost::asio::ssl::context& ssl_ctx) -> void
{
  ::SSL_CTX_set_cert_verify_callback(ssl_ctx.native_handle(),
                                     &::foxy::certify::verify_server_certificates, nullptr);
}

auto
foxy::certify::set_server_hostname(::SSL*                     ssl,
                                   boost::string_view         hostname,
                                   unsigned int               flags,
                                   boost::system::error_code& ec) -> void
{
  auto* const verify_param = ::SSL_get0_param(ssl);
  ::X509_VERIFY_PARAM_set_hostflags(verify_param, flags | X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

  if (!X509_VERIFY_PARAM_set1_host(verify_param, hostname.data(), hostname.size())) {
    ec = {static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
    return;
  }

  ec = {};
}

auto
foxy::certify::set_server_hostname(::SSL* ssl, boost::string_view hostname, unsigned int flags)
  -> void
{
  auto ec = boost::system::error_code();

  auto* const verify_param = ::SSL_get0_param(ssl);
  ::X509_VERIFY_PARAM_set_hostflags(verify_param, flags | X509_CHECK_FLAG_NO_PARTIAL_WILDCARDS);

  if (!X509_VERIFY_PARAM_set1_host(verify_param, hostname.data(), hostname.size())) {
    ec = {static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
  } else {
    ec = {};
  }

  if (ec) { boost::throw_exception(boost::system::system_error(ec)); }
}
