#include "foxy/utility.hpp"

#include <boost/spirit/home/x3.hpp>
#include <boost/fusion/container/vector.hpp>

namespace x3 = boost::spirit::x3;

namespace foxy
{

auto
parse_authority_form(boost::string_view const uri)
-> std::pair<std::string, std::string>
{
  auto host = std::string();
  auto port = std::string();

  auto       begin = uri.begin();
  auto const end   = uri.end();

  auto attr = boost::fusion::vector<std::string&, std::string&>(host, port);

  x3::parse(
    begin, end,
    +(x3::char_ - ':') >> ':' >> x3::repeat(0, 16)[x3::digit],
    attr);

  return std::make_pair(std::move(host), std::move(port));
}

} // foxy