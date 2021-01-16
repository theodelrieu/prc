#pragma once

#include <optional>

#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/utility/error_reporting.hpp>

namespace boost::spirit::x3::traits
{
template <typename T>
struct is_optional<std::optional<T>> : mpl::true_
{
};
}
