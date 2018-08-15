#include "foxy/log.hpp"

auto foxy::log_error(
  boost::system::error_code const ec,
  boost::string_view const        what) -> void
{
  std::cerr << what << " : " << ec.message() << "\n";
}