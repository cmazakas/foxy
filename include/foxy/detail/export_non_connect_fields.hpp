#ifndef FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_
#define FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_

#include <boost/beast/http/type_traits.hpp>
#include <type_traits>

namespace foxy
{
namespace detail
{

template <class Fields>
void
export_non_connect_fields(Fields& src, Fields& dst);

} // detail
} // foxy

template <class Fields>
void
foxy::detail::export_non_connect_fields(Fields& src, Fields& dst)
{
}

#endif // FOXY_DETAIL_EXPORT_NON_CONNECT_FIELDS_HPP_
