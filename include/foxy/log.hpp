#ifndef FOXY_LOG_HPP_
#define FOXY_LOG_HPP_

#include <boost/system/error_code.hpp>
#include <boost/utility/string_view.hpp>
#include <iostream>

namespace foxy
{

auto log_error(boost::system::error_code const ec, boost::string_view const what) -> void;

} // foxy

#endif // FOXY_LOG_HPP_
