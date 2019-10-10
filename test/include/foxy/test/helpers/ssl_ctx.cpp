//
// Copyright (c) 2018-2019 Christian Mazakas (christian dot mazakas at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying file LICENSE_1_0.txt
// or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/LeonineKing1199/foxy
//

#include <foxy/utility.hpp>
#include <foxy/test/helpers/ssl_ctx.hpp>
#include <boost/asio/buffer.hpp>

namespace ssl  = boost::asio::ssl;
namespace asio = boost::asio;

auto
foxy::test::make_server_ssl_ctx() -> boost::asio::ssl::context
{
  auto ctx = ssl::context(ssl::context::method::tlsv12_server);

  auto const cert = boost::string_view(
    "-----BEGIN CERTIFICATE-----\n"
    "MIIDWzCCAkMCFEkgH0LSdOVvLn/i+NRjXLmhgXjGMA0GCSqGSIb3DQEBCwUAMHsx\n"
    "CzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9ybmlhMRMwEQYDVQQHDApTYWNy\n"
    "YW1lbnRvMSAwHgYDVQQKDBdDb2RlciBpbiBhIENhdmUgU3R1ZGlvczEgMB4GA1UE\n"
    "AwwXQ29kZXIgaW4gYSBDYXZlIFN0dWRpb3MwHhcNMTkwOTE1MjM0MjQzWhcNMjQw\n"
    "OTEzMjM0MjQzWjBZMQswCQYDVQQGEwJVUzETMBEGA1UECAwKU29tZS1TdGF0ZTEh\n"
    "MB8GA1UECgwYSW50ZXJuZXQgV2lkZ2l0cyBQdHkgTHRkMRIwEAYDVQQDDAkxMjcu\n"
    "MC4wLjEwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDhXSGTS78ZuAJN\n"
    "ou764hvCOP9yY02sgCcZJ0Ykni8SK/cE0USCe1VEiWgwcedLAJgRvkEhtvinsSFk\n"
    "mUGkmaOy+dwg0lgjmuIlhsi3rA2ngprjcLVqPriwNfVBhkKb+cztwjnrNomxq/Ab\n"
    "/sX2lDAb6VbfVTNnmMT5QbOljzYf82gWc6pYcm5pixooQBD+W52ehq/jpPMYvoqY\n"
    "Vqnz/XKT8NvO3bMhbG2hgKxlocpznog61ih9N39OISbdLDbNXNrkpFfs/ST3I2mK\n"
    "5juVyQuBoJftJOHc+xtlPRcJfRMWSigGrXc4Nv61GJgBv+PwqpJpwjHe/bEc2h8D\n"
    "JJpZ1tuZAgMBAAEwDQYJKoZIhvcNAQELBQADggEBAJWIJoWb035YgXRrW8Qd0TWy\n"
    "n3DmFdN0AEaS7JVrA8zqpGNwZmShvLV4YEEasO/NHXU0qpxuoiKhAAB+e/QEutBP\n"
    "o5gopns4T3irRJYb++RDaVbJwpl1VmenSRDkg5wQT6ZoH3l4coxrD8nbF2mOz3jP\n"
    "VGuG0JhT80rGuOyHVgRz2ilRMgsBc0svPcIo9oNHFQnDCp4uArBhRp+LEK72nZ3J\n"
    "X75SCLwET3fg5QCsfvGQp9cvBqgMWYL9NhXD68xkLlzoWVoGZeM4Pe/QS9A/Eyqu\n"
    "m48CeYe3QQpC395iTjgn0eW6Y/Qaicu7znwIfaVPS+049Lp/HdKL4jFjiv2Kt/A=\n"
    "-----END CERTIFICATE-----\n");

  auto const key = boost::string_view(
    "-----BEGIN RSA PRIVATE KEY-----\n"
    "MIIEpQIBAAKCAQEA4V0hk0u/GbgCTaLu+uIbwjj/cmNNrIAnGSdGJJ4vEiv3BNFE\n"
    "gntVRIloMHHnSwCYEb5BIbb4p7EhZJlBpJmjsvncINJYI5riJYbIt6wNp4Ka43C1\n"
    "aj64sDX1QYZCm/nM7cI56zaJsavwG/7F9pQwG+lW31UzZ5jE+UGzpY82H/NoFnOq\n"
    "WHJuaYsaKEAQ/ludnoav46TzGL6KmFap8/1yk/Dbzt2zIWxtoYCsZaHKc56IOtYo\n"
    "fTd/TiEm3Sw2zVza5KRX7P0k9yNpiuY7lckLgaCX7STh3PsbZT0XCX0TFkooBq13\n"
    "ODb+tRiYAb/j8KqSacIx3v2xHNofAySaWdbbmQIDAQABAoIBAQDJ7AgJUfUHtjda\n"
    "GILHh5AXlbpLY25VAP4HK4lNhe3m+j15s4cO4jKkFfmkbmouaXnXbAAvlSF2Ht8s\n"
    "o6SNNpvV4Mm7HryaKkw2E24EI8SYMg1Ve8cQSuJv/+ifrQxBdLCI113Nwi/dYZDh\n"
    "hIUbSetRFuEfedd1GwxhyNyNmqOEO4R6dnt1rTmFhtXqHaUWbrCuTp5t3JrnQQO+\n"
    "jPgLd35G9A1hjXQ0CFUzkd1JuVJdx/U6nN4JKWc2qimQ8t1YpPQ0v8I6Q2DrPmIe\n"
    "pY7bh7wheZ2JwsJ0spgtTfUwCP3LZfQ/UW5BdjHtxnwlyuoHWLB8dgD8XfJHvwd4\n"
    "+p1HN7YBAoGBAPiIMOUk3rU2hOklLU3TvpD4/GtzhO08KR9T4XrPG4lMJ2/fx4OG\n"
    "XqYFWA1hkY7f1Lc9L1LxeOUAfNEagC7K9nz8ESVa8ink4X2F1IwIyDAU19aTvYK4\n"
    "9e4LrM9C9td6BQF0r3jemmN1qjV1naRWj4z8MlW3O2hBRo9RijOVMrQ1AoGBAOgi\n"
    "uI9v8mJXtrDeTAjVbSqnfw0Ra3WXi1ONfOXvPjldnCHCmwsv7PRpET/jvPZrWvaB\n"
    "96RpiEMcClDCaJX62zhEtCk/2Or2xvSureCnRavzmFmKF2Ehjom0ova6rImlmdWh\n"
    "u2NpfxXb/nR9mBClrElG47diW4qucHpiNo4aEa5VAoGAdAjx+yoZqLWJnGi1HC8O\n"
    "PBVjlK9ckn6SHIRHM9VaX+HkT8FFH00vB4hbMfQpx3ENmXfBjpIbBaASpnYe/rnY\n"
    "F0aAotYxVgn8lWRUdgTrojc5Bn/37P56I+fjiOkU4kmf6KwX+PDFWEZpb4g4T6/y\n"
    "WbqtrYNdAzHmxacmRSsVfzkCgYEA3cAVSEhrZdBen9SrE6E1+JIqx0QFwD51BOrb\n"
    "Dhed/FTVClcJnwU4OT6JENwvrcJeEa+T7oY1ec42eHFOUT9i3Pycke8A+2ukIScg\n"
    "yMNhxeIcfiRxMwNIU3mwVzt6CL+eFbq69DtaAHq4N3WmpvhsfU9vxsX5pp/+qJpb\n"
    "fSGgFEUCgYEA6iyrZxQ5QDIZMqzJYXsB6FW4TnAB5sEUTWzoOfsvs3iRMB/y/yhp\n"
    "pG1N/bFkTnAmwU8/Hp9ZozFXMUxb3AHnWMeg6YPhTYvUj5X1NzsCvSR3KX+V/crC\n"
    "84+kQvgny42OaKjwmvTbRCqv7/iOZzSqCSwMytcFVx11NO4+FMFSUp4=\n"
    "-----END RSA PRIVATE KEY-----\n");

  ctx.set_options(ssl::context::default_workarounds | ssl::context::no_sslv2 |
                  ssl::context::single_dh_use);

  ctx.use_certificate_chain(asio::buffer(cert.data(), cert.size()));
  ctx.use_private_key(asio::buffer(key.data(), key.size()), ssl::context::file_format::pem);

  return ctx;
}

auto
foxy::test::make_client_ssl_ctx() -> boost::asio::ssl::context
{
  auto ctx = ssl::context(ssl::context::method::tlsv12_client);

  foxy::certify::enable_https_verification(ctx);
  ctx.set_verify_mode(ssl::context::verify_peer | ssl::context::verify_fail_if_no_peer_cert);

  ctx.load_verify_file("root-cas.pem");

  // This is our root CA that was used to sign the server's certificate
  //
  auto const root_ca = boost::string_view(
    "-----BEGIN CERTIFICATE-----\n"
    "MIID1zCCAr+gAwIBAgIUBzm5/pOjqdVMVfoTQgGf8RxiuAkwDQYJKoZIhvcNAQEL\n"
    "BQAwezELMAkGA1UEBhMCVVMxEzARBgNVBAgMCkNhbGlmb3JuaWExEzARBgNVBAcM\n"
    "ClNhY3JhbWVudG8xIDAeBgNVBAoMF0NvZGVyIGluIGEgQ2F2ZSBTdHVkaW9zMSAw\n"
    "HgYDVQQDDBdDb2RlciBpbiBhIENhdmUgU3R1ZGlvczAeFw0xOTA5MTUyMzI4MDha\n"
    "Fw0yNDA5MTMyMzI4MDhaMHsxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9y\n"
    "bmlhMRMwEQYDVQQHDApTYWNyYW1lbnRvMSAwHgYDVQQKDBdDb2RlciBpbiBhIENh\n"
    "dmUgU3R1ZGlvczEgMB4GA1UEAwwXQ29kZXIgaW4gYSBDYXZlIFN0dWRpb3MwggEi\n"
    "MA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQDDBUCUPopmLtS8Zpd9zMz0+9Br\n"
    "iP4DqRfciqe9ltRBJGvFVaXpyDjknQPcl+cfKiw/4DKRDUnb6EmGk3v8PcVtPIF7\n"
    "ko59URmMelgxdhQSW6oraUGp0xE9UVsRVJYNpnnqAg0rUwwTKAKJx+7aAx75eWEn\n"
    "seWFZK7plZwGFDkvEvAgKsXDSZGjTqgcsbpWaJQ1o3XRhNcIfakhe6pA4hVlIBqM\n"
    "NgeDqCwLgvnRwqNtfFPNCcjizIngX7Jc48MhtgcGKKYo1ddUwE9lj32kB5rpk5wt\n"
    "a6fPkvWYDFu2gWDgtxxEO0bJJkFxPRbppNHxQp00h8ojk+QFDePCd9EAGGPDAgMB\n"
    "AAGjUzBRMB0GA1UdDgQWBBQIo4VHujILmom1rPijIu1U8ndLZDAfBgNVHSMEGDAW\n"
    "gBQIo4VHujILmom1rPijIu1U8ndLZDAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3\n"
    "DQEBCwUAA4IBAQBcnGgLMMRhLq0WuoedkxG/G6zuStoDiuGtKzg0TYvdgegOjpHl\n"
    "PsCQmCEUcPMFoo+ohqlZMwoLTZNv02JpRaiBGbOFFNvLQM/cyQ5neGb+o5zmoJAW\n"
    "jG5gzY1mQB3KGq+tP/IBvvdGkq7aqC3Qmnvqr5+3qO6sFzQlXGuH6Qwai8mZJhLB\n"
    "zV6lehqLWuFiWfmAe08V9jDSaMR4mQEYPdi4vkxu8L1/yI12tptdkYoTleQ+qJiy\n"
    "CGwuI6o0hcJWpOEJBkut35FagHqjL54ORLUYQ13kRIaHQkSQ8UgAEa/TbhGDqqjU\n"
    "bPbxZffpGVFTPP02ILYH3/cS6hN1vjltNk/4\n"
    "-----END CERTIFICATE-----\n");

  ctx.add_certificate_authority(asio::const_buffer(root_ca.data(), root_ca.size()));

  return ctx;
}
