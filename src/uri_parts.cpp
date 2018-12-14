#include <foxy/uri_parts.hpp>

auto
foxy::uri_parts::scheme() const -> string_view
{
  return string_view();
}

auto
foxy::uri_parts::host() const -> string_view
{
  return string_view();
}

auto
foxy::uri_parts::port() const -> string_view
{
  return string_view();
}

auto
foxy::uri_parts::path() const -> string_view
{
  return string_view();
}

auto
foxy::uri_parts::query() const -> string_view
{
  return string_view();
}

auto
foxy::uri_parts::fragment() const -> string_view
{
  return string_view();
}

auto
foxy::make_uri_parts(uri_parts::string_view const uri_view) -> uri_parts
{
  return foxy::uri_parts();
}
