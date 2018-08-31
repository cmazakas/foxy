#ifndef FOXY_UTILITY_HPP_
#define FOXY_UTILITY_HPP_

#include <boost/utility/string_view.hpp>
#include <utility>

namespace foxy
{

auto
parse_authority_form(boost::string_view const uri)
-> std::pair<std::string, std::string>;

} // foxy

#endif // FOXY_UTILITY_HPP_
